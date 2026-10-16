#include "platform/CCDevice.h"
#include "platform/CCFileUtils.h"
#include "base/ccTypes.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <vector>
#include <algorithm>
#include <cstring>
#include <stdexcept>
namespace cocos2d {
int Device::getDPI() { return 96; }
void Device::setAccelerometerEnabled(bool enabled) { if(enabled) throw std::runtime_error("Browser accelerometer is not implemented"); }
void Device::setAccelerometerInterval(float) {}
Data Device::getTextureDataForText(const char* text,const FontDefinition& def,TextAlign align,int& w,int& h,bool& premul) {
 static FT_Library library=nullptr;
 if(!library && FT_Init_FreeType(&library)) throw std::runtime_error("FreeType initialization failed");
 auto path=FileUtils::getInstance()->fullPathForFilename(def._fontName=="8-Bit Madness" ? "fonts/8-Bit-Madness.ttf" : def._fontName);
 if(path.empty()) path=FileUtils::getInstance()->fullPathForFilename("fonts/8-Bit-Madness.ttf");
 FT_Face face=nullptr;
 if(FT_New_Face(library,path.c_str(),0,&face)) throw std::runtime_error("Bundled font failed to load");
 FT_Set_Pixel_Sizes(face,0,def._fontSize);
 int advance=0;
 for(const unsigned char* c=(const unsigned char*)text;*c;c++) if(!FT_Load_Char(face,*c,FT_LOAD_DEFAULT)) advance+=face->glyph->advance.x>>6;
 w=def._dimensions.width>0?def._dimensions.width:std::max(1,advance);
 int line=face->size->metrics.height>>6;
 h=def._dimensions.height>0?def._dimensions.height:std::max(1,line);
 std::vector<unsigned char> pixels(w*h*4,0);
 int horizontal=(int)align&15,vertical=((int)align>>4)&15;
 int pen=horizontal==3?(w-advance)/2:horizontal==2?w-advance:0;
 int yOffset=vertical==3?(h-line)/2:vertical==2?h-line:0;
 int baseline=yOffset+(face->size->metrics.ascender>>6);
 for(const unsigned char* c=(const unsigned char*)text;*c;c++) {
  if(FT_Load_Char(face,*c,FT_LOAD_RENDER|FT_LOAD_TARGET_MONO)) continue;
  auto g=face->glyph; auto& b=g->bitmap;
  for(unsigned y=0;y<b.rows;y++) for(unsigned x=0;x<b.width;x++) {
   int px=pen+g->bitmap_left+x,py=baseline-g->bitmap_top+y;
   if(px<0||py<0||px>=w||py>=h) continue;
   unsigned char alpha=b.pixel_mode==FT_PIXEL_MODE_MONO?((b.buffer[y*b.pitch+x/8]&(128>>(x%8)))?255:0):b.buffer[y*b.pitch+x];
   auto i=(py*w+px)*4; pixels[i]=pixels[i+1]=pixels[i+2]=pixels[i+3]=alpha;
  }
  pen+=g->advance.x>>6;
 }
 FT_Done_Face(face); premul=true; Data data; data.copy(pixels.data(),pixels.size()); return data;
}
}
