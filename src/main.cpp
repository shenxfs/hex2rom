/// @brief bin2rom
/// @file main.cpp
/// @author shenxf 380406785@@qq.com
/// @version V1.00
///
/// 
#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>

using namespace std;

#ifdef _MSC_VER
typedef unsigned short int  uint16_t;
typedef unsigned long int   uint32_t;
typedef unsigned char       uint8_t;
#endif

/// @brief HEX record struct
typedef struct{
  uint16_t raw_addr; ///<偏移地址
  uint16_t seg_addr; ///<段地址
  uint16_t sz;       ///<HEX 数据记录长度
  uint16_t tp;       ///<HEX 数据记录类型
  char buf[256];     ///<HEX 数据记录缓冲区
}HEXRECORD;
bool getHex(HEXRECORD *pd, string &hexstr);
string trim(string &str);
void usage(int exit_code);

///@brief 显示帮助信息
///@param 返回码
///@return 无
void usage(int exit_code)
{
  cerr << "Usage : hex2rom [options] HexFile..." << endl;
  cerr << "options:" << endl;
  cerr
      << "-e <ext>  specify file extension  指定扩展名默认'.rom'" << endl
      << "-h        display this help       显示帮助信息" << endl
      << "-v        display version         显示本软件版本" << endl
      << "-n <size> specify minums size     指定最小长度默认32KB" << endl
      << "-p <pad>  specify pad byte[00]    指定填充字节默认00" << endl
      << endl;
  exit(exit_code);
}

/// @brief trim
/// @param [in] str
/// @return 
string trim(string &str)
{
  size_t beg = str.find_first_not_of(" \t");
  size_t en = str.find_last_not_of(" \t");
  string ret("");
  if (beg != string::npos)
  {
    ret = str.substr(beg, en - beg + 1);
  }
  return ret;
}

/// @brief 分析HEX记录 
/// @param [in] pd
/// @param [in] hexstr
/// @retval hex 记录正确返回真，否则返回假
bool getHex(HEXRECORD *pd, string &hexstr)
{
  bool ret = false;
  if (hexstr[0] == ':')
  {
    uint16_t rawaddr, segaddr, sz, ind = 0;
    uint16_t chk, tp, by, seg = 0, chksum = 0 ;
    size_t pos = 1, len;
    len = hexstr.size();
    stringstream ss;

    pd->raw_addr = 0;
    pd->seg_addr = 0;
    pd->sz = 0;
    pd->tp = 1;

    while (pos < len)
    {
      ss.clear();
      if (seg == 0)//记录数据长度域
      {
        ss.str(hexstr.substr(pos, 2));
        ss >> hex >> sz;
        pos += 2;
        seg = 1;
        chksum = sz;
      }
      else if (seg == 1)//偏移地址域
      {
        ss.str(hexstr.substr(pos, 4));
        ss >> hex >> rawaddr;
        pos += 4;
        chksum += rawaddr / 256;
        chksum += rawaddr % 256;
        seg = 2;
      }
      else if (seg == 2)//记录类型域
      {
        ss.str(hexstr.substr(pos, 2));
        ss >> hex >> tp;
        pos += 2;
        chksum += tp;
        seg = 3;
        if (sz == 0)
        {
          seg = 4;
        }
      }
      else if (seg == 3)//记录数据域
      {
        ss.str(hexstr.substr(pos, 2));
        ss >> hex >> by;
        pd->buf[ind] = (char)by;
        pos += 2;
        chksum += by;
        ind++;
        if (ind >= sz)
        {
          seg = 4;
        }
      }
      else if (seg == 4)//校验和域
      {
        ss.str(hexstr.substr(pos, 2));
        ss >> hex >> chk;
        chksum += chk;
        seg = 5;
      }
      else
      {
        break;
      }
    }
    if ((chksum % 256) == 0)
    {
      pd->raw_addr = rawaddr;
      pd->seg_addr = 0;
      pd->sz = sz;
      pd->tp = tp;
      if (tp == 2 || tp == 4)
      {
        segaddr = pd->buf[0] << 8;
        segaddr += pd->buf[1]; 
        pd->seg_addr = segaddr; 
      }
      else if (tp ==3 || tp == 5)
      {
        segaddr = pd->buf[0] << 8;
        segaddr += pd->buf[1]; 
        rawaddr = pd->buf[2] << 8;
        rawaddr += pd->buf[3]; 
        pd->seg_addr = segaddr; 
        pd->raw_addr = rawaddr; 
      }
      ret = true;
    }
  }
  return ret;
}

/// @brief 主函数
/// @param[in] argc
/// @param[in] argv
int main(int argc, char *argv[])
{
  ifstream fin;
  ofstream fout;
  char pbuffer[65536];
  char pads[256] = {'\x00', '\x00', '\x85', '\x86', '\x08'};
  uint16_t pad = 0;
  size_t minSize = 32768;
  string binName("");
  string hexName("");
  string extName(".rom");
  bool ext_on = false;
  bool min_on = false;
  bool pad_on = false;
  int ret = 0;

  //命令行处理
  for (int i = 1; i < argc; i++)
  {
    char *pchar = argv[i];
    if (pchar[0] == '-') //选项前导字符
    {
      switch (pchar[1])
      {
      case 'e': //指定扩展名
        ext_on = true;
        break;
      case 'h': //显示帮助信息
        usage(0);
        break;
      case 'v': //显示版本信息
        cerr << "hex2rom" << endl
             << "V1.00" << endl;
        exit(false);
        break;
      case 'n':
        min_on = true;
        break;
      case 'p': //指定填充字节
        pad_on = true;
        break;
      default:
        cerr << "unknown options" << endl;
        usage(-1);
        break;
      }
    }
    else if (pad_on) //指定填充字节
    {
      pad = (uint8_t)strtol(pchar, NULL, 16);
      pad_on = false;
    }
    else if (ext_on) //指定扩展名
    {
      extName = string(pchar);
      if (extName.find_first_of(".") == string::npos)
      {
        extName.insert(0, ".");
      }
      ext_on = false;
    }
    else if (min_on) //指定起始地址，最后出现的有效
    {
      string  ads(pchar);
      stringstream ss(ads);
      ss>>hex>>minSize;
      min_on = false;
    }
    else
    {
      hexName = string(pchar); //bin文件
      break;
    }
  }

  if (hexName.empty())
  {
    cerr<<"no hex file."<<endl;
    usage(-1);
  }
  fin.open(hexName.c_str(), ifstream::binary);
  
  if (!fin)
  {
    cerr<<"Open HEX file faile"<<endl;
    exit(-1);
  }
  memset(pbuffer , pad, 65536UL);
  if (pad == 0)
  {
    memcpy(pbuffer, pads, 256);
  }
  string one;
  uint16_t baseaddr = 0;
  uint16_t ext_addr = 0;
  uint16_t phi_ext_addr = 0;
  bool first_data = true;
  bool error = false;
  HEXRECORD hd;
  
  size_t pos=hexName.find_last_of("./\\");
  size_t es = hexName.find(extName);
  if ((es == string::npos) && (hexName[pos] == '.'))
  {
    binName = hexName.substr(0, pos) + extName;
  }
  else if (es != string::npos)
  {
    binName = hexName + ".rom";
  }
  else 
  {
    binName = hexName + extName;
  }
  fout.open(binName.c_str(), ofstream::binary);
  if (!fout)
  {
    fin.close();
    exit(-1);
  }
//  system("read -n 1 -s -p 按任意键继续");
  getline(fin, one);
  while (fin.eof() != true)
  {
    one = trim(one);
    if (!one.empty())
    {
      if (getHex(&hd, one))
      {
        if (hd.tp == 0)
        {
          if (!first_data)
          {
            while (phi_ext_addr < ext_addr)
            {
              fout.write(pbuffer, 65536UL);
              memset(pbuffer, pad, 65536UL);
              phi_ext_addr++;
            }
          }
          else
          {
            first_data = false;
            phi_ext_addr = ext_addr;
          }
          if (phi_ext_addr == ext_addr)
          {
            uint32_t addr = (uint32_t)hd.raw_addr + hd.sz;
            uint16_t base = hd.raw_addr;
            if (addr < 65536UL)
            {
              if (base >= baseaddr)
              {
                memcpy(&pbuffer[base], hd.buf, hd.sz);
                baseaddr = (uint16_t)addr;
              }
            }
            else if (addr == 65536UL)
            {
              memcpy(&pbuffer[base], hd.buf, hd.sz);
              fout.write(pbuffer, 65536UL);
              phi_ext_addr++;
              baseaddr = 0;
            }
            else
            {
              fout.write(pbuffer, base);
              size_t bytes = 65536UL - base;
              fout.write(hd.buf, bytes);
              baseaddr = hd.sz - bytes;
              memset(pbuffer, pad, 65536UL);
              memcpy(pbuffer, hd.buf, baseaddr);
              phi_ext_addr++;
            }
          }
          else
          {
            error = true;
            break;
          }
          
        }
        else if (hd.tp == 1)
        {
          break;
        }
        else if (hd.tp == 2)
        {
          ext_addr = hd.seg_addr >> 12;
        }
        else if (hd.tp == 4)
        {
          ext_addr = hd.seg_addr;
        }
        else
        {
          ;
        }            
      }
      else
      {
        error = true;
        break;
      }
    } 
    getline(fin, one);
  }
  if ((!error) && (baseaddr != 0))
  {
    if (baseaddr > 32768)
    {
      fout.write(pbuffer, baseaddr);
    }
    else
    {
      fout.write(pbuffer, 32768);
    }
        
  }
  fin.close();
  fout.close(); 
  return ret;
}