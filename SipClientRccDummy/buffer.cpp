#include "buffer.h"


Buffer::Buffer(size_t initSize) : buffer_(initSize), readerIndex_(0), writerIndex_(0)
{
}

Buffer::Buffer(const char* data, size_t size) : buffer_((size>kMiniSize)?size:kMiniSize), readerIndex_(0), writerIndex_(size)
{
	std::copy(data, data+size, buffer_.begin());
}

Buffer::Buffer(const Buffer& rhs) : readerIndex_(0), writerIndex_(0)
{
	swap(rhs);
}

void Buffer::moveData(int span)
{
	if (span == 0)
		return;

	int rindex = readerIndex_;
//	assert(rindex+span >= 0 && writerIndex_+span <= buffer_.size());

	::memmove(beginRead()+span, beginRead(), readableBytes());
	writerIndex_ += span;
	readerIndex_ += span;
}

void Buffer::makeSpaceBack(size_t len)
{
	if (writableBytes() + prependableBytes() < len )
	{
		// FIXME: move readable data
		buffer_.resize(writerIndex_+len);
	}
	else
	{
		// move readable data to the front, make space inside buffer
		int rindex = readerIndex_;
//		assert(rindex >= 0);
		moveData(-rindex);
	}
}

void Buffer::makeSpaceFront(size_t len)
{
	if (writableBytes() + prependableBytes() < len )
	{
		buffer_.resize(writerIndex_+len);
	}
	// move readable data 
	moveData(len-prependableBytes());
}

size_t Buffer::pushBack(size_t len, bool resize)
{
	if (len == 0)
		return 0;

	if (writableBytes() < len)
	{
		if (resize)
			makeSpaceBack(len);
		else len = writableBytes();
	}
	writerIndex_ += len;
	return len;
}

size_t Buffer::pushBack(unsigned char ch, size_t len, bool resize)
{
	if (len == 0)
		return 0;

	if (writableBytes() < len)
	{
		if (resize)
			makeSpaceBack(len);
		else len = writableBytes();
	}
	memset(beginWrite(), ch, len);
	writerIndex_ += len;
	return len;
}

size_t Buffer::pushBack(const char* data, size_t len, bool resize)
{
	if (data == NULL || len == 0)
		return 0;

	if (writableBytes() < len)
	{
		if (resize)
			makeSpaceBack(len);
		else len = writableBytes();
	}
	std::copy(data, data+len, beginWrite());
	writerIndex_ += len;
	return len;
}

size_t Buffer::pushFront(const char* data, size_t len, bool resize)
{
	if (data == NULL || len == 0)
		return 0;

	if (prependableBytes() < len)
	{
		if (!resize)
			return 0;
		makeSpaceFront(len);
	}
	std::copy(data, data+len, beginRead()-len);
	readerIndex_ -= len;
	return len;
}

size_t Buffer::takeBack(char* buff, size_t size)
{
	if (buff == NULL || size == 0)
		return 0;

	size_t len = (readableBytes()>size)?size:readableBytes();

	std::copy(beginWrite()-len, beginWrite(), buff);
	eraseBack(len);

	return len;
}

size_t Buffer::takeFront(char* buff, size_t size)
{
	if (buff == NULL || size == 0)
		return 0;

	size_t len = (readableBytes()>size)?size:readableBytes();

	std::copy(beginRead(), beginRead()+len, buff);
	eraseFront(len);

	return len;
}
