#include <gtest/gtest.h>

#include "recorder/details/generator.h"

#include <clocale>

using namespace gunit::recorder;
using namespace gunit;

class ParamsTestSuite : public testing::Test {};

enum class TestEnum { Test };

struct TestStruct final {
  bool operator==(const TestStruct&) const { return true; }
};

std::string produceCode(const TestStruct&, CodeSink&) {
  return {};
}

std::string produceCode(const TestEnum, CodeSink&) {
  return std::string{};
}

TEST_F(ParamsTestSuite, makeParamBool) {
  EXPECT_NO_THROW(std::get<bool>(makeParam(static_cast<bool>(10))));
}

TEST_F(ParamsTestSuite, makeParamNil) {
  EXPECT_NO_THROW(std::get<Nil>(makeParam(Nil{})));
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

TEST_F(ParamsTestSuite, makeParamUserData) {
  EXPECT_NO_THROW(
      std::get<UserDataParamPtr>(makeParam(static_cast<TestEnum>(0))));
  EXPECT_NO_THROW(std::get<UserDataParamPtr>(makeParam(TestStruct{})));
}

TEST_F(ParamsTestSuite, makeParamNumericBadCast) {
  EXPECT_THROW(
      std::get<int>(makeParam(std::numeric_limits<unsigned long>::max())),
      BadNumCast);
}

TEST_F(ParamsTestSuite, paramVisitorFloating) {
  ParamVisitor visitor;
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
  ParamVisitor visitor;

  ASSERT_EQ("12", visitor(12u));
  ASSERT_EQ("12", visitor(12));
}

TEST_F(ParamsTestSuite, paramVisitorString) {
  ParamVisitor visitor;

  ASSERT_EQ("\'str\'", visitor(std::string{"str"}));
  ASSERT_EQ("\'str\'", visitor("str"));
}

TEST_F(ParamsTestSuite, paramVisitorBoolean) {
  ParamVisitor visitor;

  ASSERT_EQ("true", visitor(true));
  ASSERT_EQ("false", visitor(false));
}
