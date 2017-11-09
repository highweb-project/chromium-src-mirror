#include "platform/wtf/build_config.h"
#include "GalleryMediaObject.h"


namespace blink {

GalleryMediaObject::GalleryMediaObject()
{
	setDescription(String(""));
	setFileName(String(""));
	setFileSize(0);
	setId(String(""));
	setTitle(String(""));
	setType(String(""));
	setContent(nullptr);
}

GalleryMediaObject::~GalleryMediaObject() {
	mContent = nullptr;
}

void GalleryMediaObject::setType(String type) {
	mType = type;
}
void GalleryMediaObject::setDescription(String description) {
	mDescription = description;
}
void GalleryMediaObject::setId(String id) {
	mId = id;
}
void GalleryMediaObject::setTitle(String title) {
	mTitle = title;
}
void GalleryMediaObject::setFileName(String fileName) {
	mFileName = fileName;
}
void GalleryMediaObject::setFileSize(long fileSize) {
	mFileSize = fileSize;
}
void GalleryMediaObject::setCreatedDate(double createdDate) {
	mCreatedDate = createdDate;
}
void GalleryMediaObject::setContent(MediaContent* content) {
	mContent = content;
}

DEFINE_TRACE(GalleryMediaObject)
{
	visitor->Trace(mContent);
}

} // namespace blink
