#include "PicoquantPTUReader.h"
#include "PicoquantHelper.h"
#include <fstream>
#include <iostream>

using namespace std;

#define READ(fs, x) fs.read(reinterpret_cast<char *>(&x), sizeof(x))

PicoquantPTUReader::PicoquantPTUReader(const std::string& filename) :
AbstractPicoquantReader(filename)
{
   readHeader();
   setTemporalResolution(8);
   
   n_x = 0;
   n_y = 0;
   determineDwellTime();
}

void PicoquantPTUReader::readHeader()
{
   ifstream fs(filename, ifstream::in | ifstream::binary);
   
   char magic[8];
   char version[8];
   
   fs.read(magic, sizeof(magic));
   
   if (string("PQTTTR") != magic)
      throw runtime_error("Wrong magic string, this is not a PTU file");

   fs.read(version, sizeof(version));

   TagHead tag_head;
   do
   {
      fs.read(reinterpret_cast<char*>(&tag_head), sizeof(tag_head));
      
      cout << "\n" << tag_head.Ident << " : ";
      
      switch (tag_head.Typ)
      {
         case tyEmpty8:
            cout << "<empty Tag>";
            break;
            
         case tyBool8:
            cout << bool(tag_head.TagValue);
            break;
            
         case tyInt8:
            cout << int(tag_head.TagValue);
            // get some Values we need to analyse records
            if (strcmp(tag_head.Ident, TTTRTagNumRecords)==0) // Number of records
               n_records = tag_head.TagValue;
            if (strcmp(tag_head.Ident, Measurement_Mode)==0) // measurement mode
               measurement_mode = tag_head.TagValue;
            if (strcmp(tag_head.Ident, HW_InpChannels)==0)
               routing_channels = tag_head.TagValue;
            break;
            
         case tyBitSet64:
            cout << tag_head.TagValue;
            break;
         
         case tyColor8:
            cout << tag_head.TagValue;
            break;
         
         case tyFloat8:
            cout << *(double*)&(tag_head.TagValue);
            if (strcmp(tag_head.Ident, TTTRTagRes)==0) // Resolution for TCSPC-Decay
               resolution = *(double*)&(tag_head.TagValue);
            //if (strcmp(tag_head.Ident, TTTRTagGlobRes)==0) // Global resolution for timetag
            //   GlobRes = *(double*)&(tag_head.TagValue); // in ns
            break;
         
         case tyTDateTime:
            break;
         
         case tyAnsiString:
         case tyFloat8Array:
         case tyWideString:
         case tyBinaryBlob:
            fs.seekg((long)tag_head.TagValue, ios_base::cur);
            break;
         
         default:
            throw runtime_error("Illegal Type identifier found! Broken file?");
      }
              
   } while (strncmp(tag_head.Ident, FileTagEnd, sizeof(FileTagEnd)));
   
   data_position = fs.tellg();
}