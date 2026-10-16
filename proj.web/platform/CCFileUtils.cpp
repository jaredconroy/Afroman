#include "platform/CCFileUtils.h"
#include <sys/stat.h>
namespace cocos2d {
class WebFileUtils : public FileUtils {
public:
 bool init() override { _defaultResRootPath="/Resources/"; mkdir("/save",0700); return FileUtils::init(); }
 std::string getWritablePath() const override { return "/save/"; }
 bool isFileExistInternal(const std::string& name) const override { struct stat s; return stat(name.c_str(),&s)==0 && S_ISREG(s.st_mode); }
};
FileUtils* FileUtils::getInstance() {
 if(!s_sharedFileUtils) { auto f=new WebFileUtils(); if(!f->init()) { delete f; return nullptr; } s_sharedFileUtils=f; }
 return s_sharedFileUtils;
}
}
