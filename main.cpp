#include "RapidJsonUtil.h"
#include <iostream>

void beforeUtils( )
{
   rapidjson::Document doc;
   auto &alloc = doc.GetAllocator();
   rapidjson::Value Array;
   Array.SetArray();
   for( int i = 0; i < 500; ++i )
   {
      rapidjson::Value val;
      val.SetInt( i );
      Array.PushBack( val, alloc );
   }
   rapidjson::Value Obj;
   Obj.SetObject();

   rapidjson::Value key( rapidjson::kStringType );
   std::string keyStr = "IntArray";
   key.SetString( keyStr.c_str(), keyStr.length(), alloc );
   Obj.AddMember( key, Array, alloc );

   rapidjson::StringBuffer buff;
   rapidjson::Writer<rapidjson::StringBuffer> writer{ buff };
   Obj.Accept( writer );
   std::cout<<"Output is: "<< buff.GetString() <<std::endl;

}

void usingUtils()
{
   JsonObject<rapidjson::Value, rapidjson::kObjectType>  Obj;
   JsonObject<rapidjson::Value, rapidjson::kArrayType>  Array;
   for( int i = 0; i < 500; ++i )
      Array.put( i );
   Obj.put(  "IntArray", Array );
   std::cout<<"Output is: "<<Obj.toString()<<std::endl;
}

void fluentInterface()
{
   JsonObject<rapidjson::Value, rapidjson::kArrayType> stringArray;
   stringArray.put( "Hello").put("World").put("Goodbye").put("World");
   std::cout<<stringArray.toString()<<std::endl;
}

int main()
{
   std::cout<<"Without Utils: \n";
   beforeUtils();
   std::cout<<"Using Utils: \n";
   usingUtils();
   std::cout<<"Fluent Interface:\n";
   fluentInterface();
   return 0;
}


