#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/writer.h>
#include <type_traits>
#include <string>

template<typename T, rapidjson::Type = rapidjson::kNullType>
struct JsonTrait{
};

template<>
struct JsonTrait<int>{
   enum{ value = rapidjson::kNumberType };
   void get( rapidjson::Value& val, int& retVal )
   {
      retVal = val.GetInt();
   }
   void put( rapidjson::Value& jval , const int& val, decltype( rapidjson::Document().GetAllocator() )& ){
      jval.SetInt( val );
   }
};


template<>
struct JsonTrait<bool>{
   enum{ value = rapidjson::kTrueType };
   void get( rapidjson::Value& val, bool& retVal )
   {
      retVal = val.GetBool();
   }
   void put( rapidjson::Value& jval , const bool& val, decltype( rapidjson::Document().GetAllocator() )& ){
      jval.SetBool( val );
   }
};

template<>
struct JsonTrait<std::string>{
   enum{ value = rapidjson::kStringType };
   void get( rapidjson::Value& val, std::string& retVal )
   {
      retVal = std::string{val.GetString()};
   }
   void put( rapidjson::Value& jval , const std::string& val,
         decltype( rapidjson::Document().GetAllocator() )& alloc){
      jval.SetString( val.c_str(), val.length(), alloc );
   }
};

template<>
struct JsonTrait<char*>{
   enum{ value = rapidjson::kStringType };
   void get( rapidjson::Value& , char*& )
   {
      //static_assert( true, "unsupported" );
   }
   void put( rapidjson::Value& jval , const char* szVal,
         decltype( rapidjson::Document().GetAllocator() )& alloc){
      if( nullptr == szVal ) return;
      std::string val{ szVal };
      jval.SetString( val.c_str(), val.length(), alloc );
   }
};

template<typename T>
struct is_json{
   static constexpr bool value = false;
};
template<>
struct is_json<rapidjson::Value>{
   static constexpr bool value = true;
};



template <typename T, rapidjson::Type U = rapidjson::kNullType>
class JsonObject
{
   static constexpr rapidjson::Type type = U;
   JsonTrait<typename std::decay<typename std::remove_cv_t<T>>::type,U>  _trait;
   rapidjson::Value& _jsonVal;
   rapidjson::Document _doc;
   decltype( rapidjson::Document().GetAllocator() )& _allocator;
public:
   JsonObject( rapidjson::Value& val, 
         decltype( rapidjson::Document().GetAllocator() )& alloc ):_jsonVal( val ),
   _allocator( alloc ){
      if( type == rapidjson::kObjectType )
         _jsonVal.SetObject();
      else if( type == rapidjson::kArrayType )
         _jsonVal.SetArray();
   }
   JsonObject():_jsonVal( _doc ),
   _allocator( _doc.GetAllocator() ){
      if( type == rapidjson::kObjectType )
         _jsonVal.SetObject();
      else if( type == rapidjson::kArrayType )
         _jsonVal.SetArray();
   }
   JsonObject( const JsonObject& obj ): 
      _jsonVal( obj._jsonVal ),
   _allocator( obj._allocator )
   {
   }

   JsonObject(rapidjson::Value& val):_jsonVal( val ),
   _allocator( _doc.GetAllocator() ){
   }

   JsonObject& operator =(const JsonObject& val){
      if( this != &val._jsonVal )
      {
         _jsonVal = val._jsonVal;
      }
      return *this;
   }
   JsonObject& operator =(rapidjson::Value& val){
      if( &this->_jsonVal != &val )
      {
         _jsonVal = val;
      }
      return *this;
   }

   template<typename V>
      JsonObject<T,U>& put(const V& val)
      {
         _trait.put( _jsonVal, val, _allocator );
         return *this;
      }

   template<typename V>
      JsonObject<T,U>& put( const std::string& key, const V& val )
      {
         _trait.put( _jsonVal, key, val, _allocator );
         return *this;
      }

   template<typename V>
      JsonObject<T,U>& get(V& val)
      {
         _trait.get( _jsonVal, val );
         return *this;
      }

   template<typename V>
      JsonObject<T,U>& get(const std::string& key, V& val)
      {
         _trait.get( _jsonVal, key, val );
         return *this;
      }


   operator rapidjson::Value&()
   {
      return _jsonVal;
   }

   operator const rapidjson::Value&()const
   {
      return _jsonVal;
   }

   std::string toString()
   {
      rapidjson::StringBuffer buff;
      rapidjson::Writer<rapidjson::StringBuffer> writer{ buff };
      _jsonVal.Accept( writer );
      return std::string{ buff.GetString() };
   }

};

template<typename T, rapidjson::Type U>
struct is_json<JsonObject<T,U>>{
   static constexpr bool value = true;
};


template<>
struct JsonTrait<rapidjson::Value, rapidjson::kObjectType>{
   enum{ value = rapidjson::kObjectType };

   template <typename T, typename X = typename std::enable_if<!is_json<typename std::remove_cv<T>::type>::value>::type >
      void get( rapidjson::Value& val, const std::string& key, T& outVal )
      {
         JsonTrait<typename std::remove_cv_t<T>> trait;
         auto it = val.FindMember( key.c_str() );
         if( it != val.MemberEnd() )
         {
            trait.get( it->value, outVal );
         }
      }

   template <typename T, typename X = typename std::enable_if<is_json<typename std::remove_cv<T>::type>::value>::type, typename Y = void>
      void get( rapidjson::Value& jVal, const std::string& key, T& outVal)
      {
         auto it = jVal.FindMember( key.c_str() );
         outVal = it->value;
      }

   template<typename T, typename X = typename std::enable_if<is_json<typename std::remove_cv<T>::type>::value>::type>
      void put( rapidjson::Value& jval,
            const std::string& key,
            const T& val,
            decltype( rapidjson::Document().GetAllocator() )& alloc )
      {
         JsonTrait<std::string> stringType;
         rapidjson::Value jKey;
         stringType.put( jKey, key, alloc );
         const rapidjson::Value& temp = static_cast<const rapidjson::Value&>( val );
         jval.AddMember( jKey, const_cast<rapidjson::Value&>(temp), alloc );
      }

   template<typename T, typename X = typename std::enable_if<!is_json<typename std::remove_cv<T>::type>::value>::type, typename Y = void>
      void put( rapidjson::Value& jval, 
            const std::string& key,
            const T& val,
            decltype( rapidjson::Document().GetAllocator() )& alloc )
      {
         JsonTrait<std::string> stringType;
         rapidjson::Value jKey,valObj;
         stringType.put( jKey, key, alloc );
         JsonTrait<T> valTrait;
         valTrait.put( valObj, val, alloc );
         jval.AddMember( jKey, valObj, alloc );
      }


};

template<>
struct JsonTrait<rapidjson::Value, rapidjson::kArrayType>{

   template<typename T, typename X =  std::enable_if_t<!is_json<typename std::remove_cv<T>::type>::value>>
   void put( rapidjson::Value& jval, T& input,
         decltype( rapidjson::Document().GetAllocator() )& alloc )
   {
      JsonTrait<typename std::decay<typename std::remove_cv_t<T>>::type> trait;
      rapidjson::Value inputJVal;
      trait.put( inputJVal, input, alloc);
      jval.PushBack( inputJVal, alloc );
   }

   template<typename T, typename X =  typename std::enable_if<is_json<typename std::remove_cv<T>::type>::value>::type, typename Y = void >
   void put( rapidjson::Value& jval,
         const T& input,
         decltype( rapidjson::Document().GetAllocator() )& alloc )
   {
      JsonTrait<typename std::decay<typename std::remove_cv_t<T>>::type> trait;
      jval.PushBack( const_cast<T&>(input), alloc );
   }
};



template<>
struct JsonTrait<JsonObject<rapidjson::Value, rapidjson::kObjectType>>: JsonTrait<rapidjson::Value, rapidjson::kObjectType>
{
};

template<>
struct JsonTrait<JsonObject<rapidjson::Value, rapidjson::kArrayType>>: JsonTrait<rapidjson::Value, rapidjson::kArrayType>
{
};

