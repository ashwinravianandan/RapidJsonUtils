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
   enum{ value = rapidjson::kTrueType };
   void get( rapidjson::Value& val, std::string& retVal )
   {
      retVal = std::string{val.GetString()};
   }
   void put( rapidjson::Value& jval , const std::string& val,
         decltype( rapidjson::Document().GetAllocator() )& alloc){
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

template<>
struct JsonTrait<rapidjson::Value, rapidjson::kObjectType>{
   enum{ value = rapidjson::kObjectType };

   template <typename T, typename X = typename std::enable_if<!is_json<typename std::remove_cv<T>::type>::value>::type >
      void get( rapidjson::Value& val, const std::string& key, const T& outVal )
      {
         JsonTrait<std::remove_cv<T>> trait;
         auto it = val.FindMember( key.c_str() );
         if( it != val.MemberEnd() )
         {
            trait.get( it->value, outVal );
         }
      }

   template <typename T, typename X = typename std::enable_if<is_json<typename std::remove_cv<T>::type>::value>::type>
      void get( rapidjson::Value& , std::string& , const T&)
      {
         //static_assert( false, "Do you really want to read members into Json" );
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
         jval.AddMember( jKey, const_cast<rapidjson::Value&>(val), alloc );
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
   enum{ value = rapidjson::kArrayType };

   template<typename T, typename X =  std::enable_if_t<!is_json<typename std::remove_cv<T>::type>::value>>
   void put( rapidjson::Value& jval, T& input,
         decltype( rapidjson::Document().GetAllocator() )& alloc )
   {
      JsonTrait<typename std::remove_cv<T>::type> trait;
      rapidjson::Value inputJVal;
      trait.put( inputJVal, input, alloc);
      jval.PushBack( inputJVal, alloc );
   }

   template<typename T, typename X =  typename std::enable_if<is_json<typename std::remove_cv<T>::type>::value>::type, typename Y = void >
   void put( rapidjson::Value& jval,
         const T& input,
         decltype( rapidjson::Document().GetAllocator() )& alloc )
   {
      JsonTrait<typename std::remove_cv<T>::type> trait;
      jval.PushBack( const_cast<T&>(input), alloc );
   }
};

template <typename T, rapidjson::Type U = rapidjson::kNullType>
class JsonObject
{
   static constexpr rapidjson::Type type = U;
   JsonTrait<T,U>  _trait;
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

   //template<typename V, typename X = typename std::enable_if<!is_json<typename std::remove_cv<V>::type>::value>::type>
   template<typename V>
      void put(const V& val)
      {
         _trait.put( _jsonVal, val, _allocator );
      }

   template<typename V>
      void put( const std::string& key, const V& val )
      {
         _trait.put( _jsonVal, key, val, _allocator );
      }

   operator rapidjson::Value&()
   {
      return _jsonVal;
   }
   operator std::string()
   {
      rapidjson::StringBuffer buff;
      rapidjson::Writer<rapidjson::StringBuffer> writer{ buff };
      _jsonVal.Accept( writer );
      return std::string{ buff.GetString() };
   }
};

