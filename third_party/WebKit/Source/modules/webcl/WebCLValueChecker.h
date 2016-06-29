/*
 * WebCLValueChecker.h
 *
 *  Created on: 2015. 7. 6.
 *      Author: azureskybox
 */

#ifndef WebCLValueChecker_h
#define WebCLValueChecker_h

#include "WebCLInclude.h"

namespace blink {

class WebCLImageDescriptor;

class WebCLValueChecker {
public:
	//image format
	static bool isImageChannelOrderValid(unsigned channelOrder);
	static bool isImageChannelTypeValid(unsigned channelType);
	static bool isImageDescriptorValid(WebCLImageDescriptor descriptor);

	//device type
	static bool isDeviceTypeValid(int deviceType);

	//command queue proeprty
	static bool isCommandQueuePropertyValid(int property);

	//command queue info name
	static bool isCommandQueueInfoNameValid(int name);
};

}

#endif
