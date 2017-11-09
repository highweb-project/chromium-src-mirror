// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 INFRAWARE, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef DeviceMediaObject_h
#define DeviceMediaObject_h

#include "platform/heap/Handle.h"
#include "platform/wtf/Forward.h"
#include "platform/wtf/RefCounted.h"
#include "public/platform/WebString.h"
#include "platform/wtf/text/WTFString.h"
#include "platform/bindings/ScriptWrappable.h"
#include "MediaContent.h"

namespace blink {

class GalleryMediaObject final : public GarbageCollectedFinalized<GalleryMediaObject>, public ScriptWrappable {
	DEFINE_WRAPPERTYPEINFO();
public:
	static GalleryMediaObject* Create() {return new GalleryMediaObject();};

	String type() const {return mType;}
	String description() const {return mDescription;}
	String id() const {return mId;}
	String title() const {return mTitle;}
	String fileName() const {return mFileName;}
	long fileSize(){return mFileSize;}
	double createdDate(){return mCreatedDate;}
	MediaContent* content(){return mContent;}

	void setType(String type);
	void setDescription(String description);
	void setId(String id);
	void setTitle(String title);
	void setFileName(String fileName);
	void setFileSize(long fileSize);
	void setCreatedDate(double createdDate);
	void setContent(MediaContent* content);

	GalleryMediaObject();
	virtual ~GalleryMediaObject();

	DECLARE_VIRTUAL_TRACE();

private:
	String mType;
	String mDescription;
	String mId;
	String mTitle;
	String mFileName;
	long mFileSize;
	double mCreatedDate;
	Member<MediaContent> mContent;
};

} // namespace blink

#endif // DeviceMediaObject_h