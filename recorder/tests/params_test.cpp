#include <gtest/gtest.h>

#include "recorder/details/lua/lua_params.h"

#include <clocale>

using namespace cider::recorder;
using namespace cider;

class ParamsTestSuite : public testing::Test {};

enum class TestEnum { Test };

struct TestStruct final {
  bool operator==(const TestStruct&) const { return true; }
};

class TestClass final {
 public:
  TestClass() {}
};

std::string produceAggregateCode(const TestStruct&, CodeSink&) {
  return {};
}

std::string produceAggregateCode(const TestEnum, CodeSink&) {
  return std::string{};
}

TEST_F(ParamsTestSuite, makeParamBool) {
  EXPECT_NO_THROW(std::get<bool>(makeParam(static_cast<bool>(10))));
}

TEST_F(ParamsTestSuite, makeParamNil) {
  EXPECT_NO_THROW(std::get<Nil>(makeParam(Nil{})));
  EXPECT_NO_THROW(std::get<Nil>(makeParam(std::nullopt)));
}

TEST_F(ParamsTestSuite, makeParamIntegral) {
  EXPECT_NO_THROW(std::get<int>(makeParam(static_cast<char>(10))));
  EXPECT_NO_THROW(std::get<int>(makeParam(static_cast<unsigned char>(10))));
  EXPECT_NO_THROW(std::get<int>(makeParam(static_cast<int>(10))));
  EXPECT_NO_THROW(std::get<int>(makeParam(static_cast<unsigned int>(10))));
  EXPECT_NO_THROW(std::get<int>(makeParam(static_cast<short>(10))));
  EXPECT_NO_THROW(std::get<int>(makeParam(static_cast<unsigned short>(10))));
  EXPECT_NO_THROW(std::get<int>(makeParam(static_cast<long>(10))));
  EXPECT_NO_THROW(std::get<int>(makeParam(static_cast<unsigned long>(10))));
}

TEST_F(ParamsTestSuite, makeParamFloating) {
  EXPECT_NO_THROW(std::get<float>(makeParam(static_cast<float>(10))));
  EXPECT_NO_THROW(std::get<float>(makeParam(static_cast<double>(10))));
}

TEST_F(ParamsTestSuite, makeParamString) {
  EXPECT_NO_THROW(std::get<std::string>(makeParam(std::string{"str"})));
  EXPECT_NO_THROW(std::get<std::string>(makeParam("str")));
}

TEST_F(ParamsTestSuite, makeValueParamUserData) {
  EXPECT_NO_THROW(
      std::get<UserDataValueParamPtr>(makeParam(static_cast<TestEnum>(0))));
  EXPECT_NO_THROW(std::get<UserDataValueParamPtr>(makeParam(TestStruct{})));
}

TEST_F(ParamsTestSuite, makeValueParamUserDataPtr) {
  TestEnum testEnum;
  EXPECT_NO_THROW(std::get<UserDataValueParamPtr>(makeParam(&testEnum)));

  auto testStruct = TestStruct{};
  EXPECT_NO_THROW(std::get<UserDataValueParamPtr>(makeParam(&testStruct)));
}

TEST_F(ParamsTestSuite, makeReferenceParamUserData) {
  TestClass object;
  EXPECT_NO_THROW(std::get<UserDataReferenceParamPtr>(makeParam(object)));
}

TEST_F(ParamsTestSuite, makeReferenceParamUserDataPtr) {
  TestClass object;
  EXPECT_NO_THROW(std::get<UserDataReferenceParamPtr>(makeParam(&object)));
}

TEST_F(ParamsTestSuite, makeParamLongNumericBadCast) {
  EXPECT_THROW(
      std::get<int>(makeParam(std::numeric_limits<unsigned long>::max())),
      BadNumCast);
}

TEST_F(ParamsTestSuite, makeParamDoubleNumericBadCast) {
  EXPECT_THROW(std::get<int>(makeParam(std::numeric_limits<double>::max())),
               BadNumCast);
  try {
    std::get<int>(makeParam(std::numeric_limits<double>::max()));
  } catch (const BadNumCast& err) {
    EXPECT_NE("", err.what());
  }
}

TEST_F(ParamsTestSuite, paramVisitorFloating) {
  lua::ParamVisitor visitor;
  const auto oldLocale = std::setlocale(LC_NUMERIC, nullptr);
  // Some platforms may not have the "De_de" locale.
  if (std::setlocale(LC_NUMERIC, "DE_de.utf-8")) {
    // EXPECT_THAT(std::to_string(12.34f), testing::StartsWith("12,34"));
    EXPECT_EQ("12.34", visitor(12.34f));  // Don't use ASSERT_, even scope guard
                                          // won't set `oldLocale` back.
    std::setlocale(LC_NUMERIC, oldLocale);
  }
}

TEST_F(ParamsTestSuite, paramVisitorInteger) {
  lua::ParamVisitor visitor;

  ASSERT_EQ("12", visitor(12u));
  ASSERT_EQ("12", visitor(12));
}

TEST_F(ParamsTestSuite, paramVisitorString) {
  lua::ParamVisitor visitor;

  ASSERT_EQ("\'str\'", visitor(std::string{"str"}));
  ASSERT_EQ("\'str\'", visitor("str"));
}

TEST_F(ParamsTestSuite, paramVisitorBoolean) {
  lua::ParamVisitor visitor;

  ASSERT_EQ("true", visitor(true));
  ASSERT_EQ("false", visitor(false));
}

TEST_F(ParamsTestSuite, paramVisitorNil) {
  lua::ParamVisitor visitor;

  ASSERT_EQ("nil", visitor(Nil{}));
}

TEST_F(ParamsTestSuite, stringParamEscapeTest) {
  lua::ParamVisitor visitor;

  ASSERT_EQ("\'str\\n\'", visitor("str\n"));
  ASSERT_EQ("\'str\\'\'", visitor("str\'"));
  ASSERT_EQ("\'str\\r\'", visitor("str\r"));
  ASSERT_EQ("\'str\\t\'", visitor("str\t"));
  ASSERT_EQ("\'str\\\"\'", visitor("str\""));
  ASSERT_EQ("\'str\\\\\\\\\'", visitor("str\\\\"));
  ASSERT_EQ("\'str\\a\'", visitor("str\a"));
  ASSERT_EQ("\'str\\b\'", visitor("str\b"));
  ASSERT_EQ("\'str\\v\'", visitor("str\v"));
  ASSERT_EQ("\'str\\f\'", visitor("str\f"));
  ASSERT_EQ("\'str\\e\'", visitor("str\e"));
}
