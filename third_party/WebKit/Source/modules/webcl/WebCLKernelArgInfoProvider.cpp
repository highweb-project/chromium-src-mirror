// Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
// Copyright (C) 2015 Intel Corporation All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/wtf/build_config.h"

#include "WebCLKernelArgInfoProvider.h"
#include "modules/webcl/WebCLKernelArgInfo.h"
#include "WebCLProgram.h"
#include "WebCLKernel.h"

namespace blink {

const size_t notFound = static_cast<size_t>(-1);

static bool isASCIILineBreakCharacter(UChar c)
{
	return c == '\r' || c == '\n';
}

inline bool isEmptySpace(UChar c)
{
	return c <= ' ' && (c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f');
}

inline bool isStarCharacter(UChar c)
{
	return c == '*';
}

inline bool isPrecededByUnderscores(const String& string, size_t index)
{
	size_t start = index - 2;
	return (/*start >= 0 &&*/ string[start + 1] == '_' && string[start] == '_');
}

static void removeComments(const String& inSource, String& outSource)
{
	enum Mode { DEFAULT, BLOCK_COMMENT, LINE_COMMENT };
	Mode currentMode = DEFAULT;

	outSource = inSource;
	for (unsigned i = 0; i < outSource.length(); ++i) {
		if (currentMode == BLOCK_COMMENT) {
			if (outSource[i] == '*' && outSource[i + 1] == '/') {
				outSource.replace(i++, 2, "  ");
				currentMode = DEFAULT;
				continue;
			}
			outSource.replace(i, 1, " ");
			continue;
		}

		if (currentMode == LINE_COMMENT) {
			if (outSource[i] == '\n' || outSource[i] == '\r') {
				currentMode = DEFAULT;
				continue;
			}
			outSource.replace(i, 1, " ");
			continue;
		}

		if (outSource[i] == '/') {
			if (outSource[i + 1] == '*') {
				outSource.replace(i++, 2, "  ");
				currentMode = BLOCK_COMMENT;
				continue;
			}

			if (outSource[i + 1] == '/') {
				outSource.replace(i++, 2, "  ");
				currentMode = LINE_COMMENT;
				continue;
			}
		}
	}
}

WebCLKernelArgInfoProvider::WebCLKernelArgInfoProvider(WebCLKernel* kernel)
	: mKernel(kernel)
{
	if (!kernel) {
		LOG(ERROR) << "kernel is null";
		return;
	}
	ensureInfo();
}

void WebCLKernelArgInfoProvider::ensureInfo()
{
	CLLOG(INFO) << "ensureInfo()";
	if (mArgumentInfoVector.size())
		return;

	String source;
	removeComments(mKernel->getProgram()->getSource(), source);
	// 0) find "kernel" string.
	// 1) Check if it is a valid kernel declaration.
	// 2) find the first open braces past "kernel".
	// 3) reverseFind the given kernel name string.
	// 4) if not found go back to (1)
	// 5) if found, parse its argument list.

	size_t kernelNameIndex = 0;
	size_t kernelDeclarationIndex = 0;
	for (size_t startIndex = 0; ; startIndex = kernelDeclarationIndex + 6) {
		kernelDeclarationIndex = source.Find("kernel", startIndex);
		if (kernelDeclarationIndex == notFound) {
			return;
		}

		// Check if "kernel" is not a substring of a valid token,
		// e.g. "akernel" or "__kernel_":
		// 1) After "kernel" there has to be an empty space.
		// 2) Before "kernel" there has to be either:
		// 2.1) two underscore characters or
		// 2.2) none, i.e. "kernel" is the first string in the program source or
		// 2.3) an empty space.
		if (!isEmptySpace(source[kernelDeclarationIndex + 6]))
			continue;

		// If the kernel declaration is not at the beginning of the program.
		bool hasTwoUnderscores = isPrecededByUnderscores(source, kernelDeclarationIndex);
		bool isKernelDeclarationAtBeginning = hasTwoUnderscores ? (kernelDeclarationIndex == 2) : (kernelDeclarationIndex == 0);

		if (!isKernelDeclarationAtBeginning) {
			size_t firstPrecedingIndex = kernelDeclarationIndex - (hasTwoUnderscores ? 3 : 1);
			if (!isEmptySpace(source[firstPrecedingIndex]))
				continue;
		}

		size_t openBrace = source.Find("{", kernelDeclarationIndex + 6);
		kernelNameIndex = source.ReverseFind(mKernel->getKernelName(), openBrace);

		if (kernelNameIndex < kernelDeclarationIndex)
			continue;

		if (kernelNameIndex != notFound)
			break;
	}

	size_t requiredIndex = source.ReverseFind("required_work_group_size", kernelNameIndex);
	if (requiredIndex != notFound) {
		size_t requiredOpenBracket = source.Find("(", requiredIndex);
		size_t requiredCloseBracket = source.Find(")", requiredOpenBracket);
		const String& requiredArgumentListStr = source.Substring(requiredOpenBracket + 1, requiredCloseBracket - requiredOpenBracket - 1);

		Vector<String> requiredArgumentStrVector;
		requiredArgumentListStr.Split(",", requiredArgumentStrVector);
		for (auto requiredArgument : requiredArgumentStrVector) {
			requiredArgument = requiredArgument.RemoveCharacters(isASCIILineBreakCharacter);
			requiredArgument = requiredArgument.StripWhiteSpace();
			mRequiredArgumentVector.push_back(requiredArgument.ToUInt());
		}
	}

	size_t openBracket = source.Find("(", kernelNameIndex);
	size_t closeBracket = source.Find(")", openBracket);
	const String& argumentListStr = source.Substring(openBracket + 1, closeBracket - openBracket - 1);

	Vector<String> argumentStrVector;
	argumentListStr.Split(",", argumentStrVector);
	for (auto argument : argumentStrVector) {
		argument = argument.RemoveCharacters(isASCIILineBreakCharacter);
		argument = argument.StripWhiteSpace();
		parseAndAppendDeclaration(argument);
	}
	CLLOG(INFO) << "ensureInfo end";
}

static void prependUnsignedIfNeeded(Vector<String>& declarationStrVector, String& type)
{
	for (size_t i = 0; i < declarationStrVector.size(); i++) {
		static AtomicString& Unsigned = *new AtomicString("unsigned"/*, AtomicString::ConstructFromLiteral*/);
		if (declarationStrVector[i] == Unsigned) {
			type = "u" + type;
			declarationStrVector.erase(i);
			return;
		}
	}
}

void WebCLKernelArgInfoProvider::parseAndAppendDeclaration(const String& argumentDeclaration)
{
	// "*" is used to indicate pointer data type, setting isPointerType flag if "*" is present in argumentDeclaration.
	// Since we parse only valid & buildable OpenCL kernels, * in argumentDeclaration must be associated with type only.
	bool isPointerType = false;
	if (argumentDeclaration.Contains("*"))
		isPointerType = true;

	Vector<String> declarationStrVector;
	argumentDeclaration.RemoveCharacters(isStarCharacter).Split(" ", declarationStrVector);

	const String& name = extractName(declarationStrVector);
	const String& addressQualifier = extractAddressQualifier(declarationStrVector);
	String type = extractType(declarationStrVector);

	static AtomicString& image2d_t = *new AtomicString("image2d_t"/*, AtomicString::ConstructFromLiteral*/);
	const String& accessQualifier = (type == image2d_t) ? extractAccessQualifier(declarationStrVector) : "none";
	prependUnsignedIfNeeded(declarationStrVector, type);

	WebCLKernelArgInfo info = WebCLKernelArgInfo();
	info.setAccessQualifier(accessQualifier);
	info.setAddressQualifier(addressQualifier);
	info.setName(name);
	info.setTypeName(type);
	info.setIsBuffer(isPointerType);

	mArgumentInfoVector.push_back(info);
}

String WebCLKernelArgInfoProvider::extractAddressQualifier(Vector<String>& declarationStrVector)
{
	static AtomicString* __Private = new AtomicString("__private"/*, AtomicString::ConstructFromLiteral*/);
	static AtomicString* Private = new AtomicString("private"/*, AtomicString::ConstructFromLiteral*/);

	static AtomicString* __Global = new AtomicString("__global"/*, AtomicString::ConstructFromLiteral*/);
	static AtomicString* Global = new AtomicString("global"/*, AtomicString::ConstructFromLiteral*/);

	static AtomicString* __Constant = new AtomicString("__constant"/*, AtomicString::ConstructFromLiteral*/);
	static AtomicString* Constant = new AtomicString("constant"/*, AtomicString::ConstructFromLiteral*/);

	static AtomicString* __Local = new AtomicString("__local"/*, AtomicString::ConstructFromLiteral*/);
	static AtomicString* Local = new AtomicString("local"/*, AtomicString::ConstructFromLiteral*/);

	String address = *Private;
	size_t i = 0;
	for (; i < declarationStrVector.size(); i++) {
		const String& candidate = declarationStrVector[i];
		if (candidate == *__Private || candidate == *Private) {
			break;
		} else if (candidate == *__Global || candidate == *Global) {
			address = *Global;
			break;
		} else if (candidate == *__Constant || candidate == *Constant) {
			address = *Constant;
			break;
		} else if (candidate == *__Local || candidate == *Local) {
			address = *Local;
			break;
		}
	}

	if (i < declarationStrVector.size())
		declarationStrVector.erase(i);

	return address;
}

String WebCLKernelArgInfoProvider::extractAccessQualifier(Vector<String>& declarationStrVector)
{
	static AtomicString* __read_only = new AtomicString("__read_only"/*, AtomicString::ConstructFromLiteral*/);
	static AtomicString* read_only = new AtomicString("read_only"/*, AtomicString::ConstructFromLiteral*/);

	static AtomicString* __write_only = new AtomicString("__read_only"/*, AtomicString::ConstructFromLiteral*/);
	static AtomicString* write_only = new AtomicString("write_only"/*, AtomicString::ConstructFromLiteral*/);

	static AtomicString* __read_write = new AtomicString("__read_write"/*, AtomicString::ConstructFromLiteral*/);
	static AtomicString* read_write = new AtomicString("read_write"/*, AtomicString::ConstructFromLiteral*/);

	String access = *read_only;
	size_t i = 0;
	for (; i < declarationStrVector.size(); i++) {
		const String& candidate = declarationStrVector[i];
		if (candidate == *__read_only  || candidate == *read_only) {
			break;
		} else if (candidate == *__write_only || candidate == *write_only) {
			access = *write_only;
			break;
		} else if (candidate == *__read_write || candidate == *read_write) {
			access = *read_write;
			break;
		}
	}

	if (i < declarationStrVector.size())
		declarationStrVector.erase(i);

	return access;
}

String WebCLKernelArgInfoProvider::extractName(Vector<String>& declarationStrVector)
{
	String last = "";
	if (declarationStrVector.size() > 0) {
		last = declarationStrVector.back();
		declarationStrVector.pop_back();
	}
	return last;
}

String WebCLKernelArgInfoProvider::extractType(Vector<String>& declarationStrVector)
{
	String type = "";
	if (declarationStrVector.size() > 0) {
		type = declarationStrVector.back();
		declarationStrVector.pop_back();
	}
	return type;
}

void WebCLKernelArgInfoProvider::clear() {
	mKernel = NULL;
	mArgumentInfoVector.clear();
	mRequiredArgumentVector.clear();
}

DEFINE_TRACE(WebCLKernelArgInfoProvider) {
	visitor->Trace(mKernel);
	visitor->Trace(mArgumentInfoVector);
	//visitor->Trace(mRequiredArgumentVector);
}

} // namespace blink
