#include <vector>		//for std::vector
#include <string>		//for std::string

struct EncodedFollowerCountInfo {
	long followerCount;
};

EncodedFollowerCountInfo EncodeFollowerCount(long follower_count) {
	EncodedFollowerCountInfo ret;
	ret.followerCount = follower_count;
	return ret;
}

long DecodeFollowerCount(EncodedFollowerCountInfo info) {
    return info.followerCount;
}

typedef enum { PROT_EXIT=FirstApplicationTag, 
               PROT_STARTPROC
             } Protocol;
