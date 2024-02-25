#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_DEPRECATE

#include "test.hpp"

#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "helpers.hpp"

using namespace pugi::PugixmlHooked;

#ifdef PUGIXML_NO_STL
template <typename I>
static I move_iter(I base, int n) {
  if (n > 0)
    while (n--)
      ++base;
  else
    while (n++)
      --base;
  return base;
}
#else
template <typename I>
static I move_iter(I base, int n) {
  std::advance(base, n);
  return base;
}
#endif

TEST_XML(dom_attr_bool_ops, "<node attr='1'/>") {
  generic_bool_ops_test(doc.child(STR("node")).attribute(STR("attr")));
}

TEST_XML(dom_attr_eq_ops, "<node attr1='1' attr2='2'/>") {
  generic_eq_ops_test(doc.child(STR("node")).attribute(STR("attr1")),
                      doc.child(STR("node")).attribute(STR("attr2")));
}

TEST_XML(dom_attr_empty, "<node attr='1'/>") {
  generic_empty_test(doc.child(STR("node")).attribute(STR("attr")));
}

TEST_XML(dom_attr_next_previous_attribute, "<node attr1='1' attr2='2' />") {
  xml_attribute attr1 = doc.child(STR("node")).attribute(STR("attr1"));
  xml_attribute attr2 = doc.child(STR("node")).attribute(STR("attr2"));

  CHECK(attr1.next_attribute() == attr2);
  CHECK(attr2.next_attribute() == xml_attribute());

  CHECK(attr1.previous_attribute() == xml_attribute());
  CHECK(attr2.previous_attribute() == attr1);

  CHECK(xml_attribute().next_attribute() == xml_attribute());
  CHECK(xml_attribute().previous_attribute() == xml_attribute());
}

TEST_XML(dom_attr_name_value, "<node attr='1'/>") {
  xml_attribute attr = doc.child(STR("node")).attribute(STR("attr"));

  CHECK_NAME_VALUE(attr, STR("attr"), STR("1"));
  CHECK_NAME_VALUE(xml_attribute(), STR(""), STR(""));
}

TEST_XML(dom_attr_as_string, "<node attr='1'/>") {
  xml_attribute attr = doc.child(STR("node")).attribute(STR("attr"));

  CHECK_STRING(attr.as_string(), STR("1"));
  CHECK_STRING(xml_attribute().as_string(), STR(""));
}

TEST_XML(dom_attr_as_int,
         "<node attr1='1' attr2='-1' attr3='-2147483648' attr4='2147483647' "
         "attr5='0'/>") {
  xml_node node = doc.child(STR("node"));

  CHECK(xml_attribute().as_int() == 0);
  CHECK(node.attribute(STR("attr1")).as_int() == 1);
  CHECK(node.attribute(STR("attr2")).as_int() == -1);
  CHECK(node.attribute(STR("attr3")).as_int() == -2147483647 - 1);
  CHECK(node.attribute(STR("attr4")).as_int() == 2147483647);
  CHECK(node.attribute(STR("attr5")).as_int() == 0);
}

TEST_XML(dom_attr_as_int_hex,
         "<node attr1='0777' attr2='0x5ab' attr3='0XFf' attr4='-0x20' "
         "attr5='-0x80000000' attr6='0x'/>") {
  xml_node node = doc.child(STR("node"));

  CHECK(node.attribute(STR("attr1")).as_int() ==
        777);  // no octal support! intentional
  CHECK(node.attribute(STR("attr2")).as_int() == 1451);
  CHECK(node.attribute(STR("attr3")).as_int() == 255);
  CHECK(node.attribute(STR("attr4")).as_int() == -32);
  CHECK(node.attribute(STR("attr5")).as_int() == -2147483647 - 1);
  CHECK(node.attribute(STR("attr6")).as_int() == 0);
}

TEST_XML(dom_attr_as_uint,
         "<node attr1='0' attr2='1' attr3='2147483647' attr4='4294967295' "
         "attr5='0'/>") {
  xml_node node = doc.child(STR("node"));

  CHECK(xml_attribute().as_uint() == 0);
  CHECK(node.attribute(STR("attr1")).as_uint() == 0);
  CHECK(node.attribute(STR("attr2")).as_uint() == 1);
  CHECK(node.attribute(STR("attr3")).as_uint() == 2147483647);
  CHECK(node.attribute(STR("attr4")).as_uint() == 4294967295u);
  CHECK(node.attribute(STR("attr5")).as_uint() == 0);
}

TEST_XML(dom_attr_as_uint_hex,
         "<node attr1='0777' attr2='0x5ab' attr3='0XFf' attr4='0x20' "
         "attr5='0xFFFFFFFF' attr6='0x'/>") {
  xml_node node = doc.child(STR("node"));

  CHECK(node.attribute(STR("attr1")).as_uint() ==
        777);  // no octal support! intentional
  CHECK(node.attribute(STR("attr2")).as_uint() == 1451);
  CHECK(node.attribute(STR("attr3")).as_uint() == 255);
  CHECK(node.attribute(STR("attr4")).as_uint() == 32);
  CHECK(node.attribute(STR("attr5")).as_uint() == 4294967295u);
  CHECK(node.attribute(STR("attr6")).as_uint() == 0);
}

TEST_XML(
    dom_attr_as_integer_space,
    "<node attr1=' \t1234' attr2='\t 0x123' attr3='- 16' attr4='- 0x10'/>") {
  xml_node node = doc.child(STR("node"));

  CHECK(node.attribute(STR("attr1")).as_int() == 1234);
  CHECK(node.attribute(STR("attr2")).as_int() == 291);
  CHECK(node.attribute(STR("attr3")).as_int() == 0);
  CHECK(node.attribute(STR("attr4")).as_int() == 0);

#ifdef PUGIXML_HAS_LONG_LONG
  CHECK(node.attribute(STR("attr1")).as_llong() == 1234);
#endif
}

TEST_XML(dom_attr_as_float,
         "<node attr1='0' attr2='1' attr3='0.12' attr4='-5.1' attr5='3e-4' "
         "attr6='3.14159265358979323846'/>") {
  xml_node node = doc.child(STR("node"));

  CHECK(xml_attribute().as_float() == 0);
  CHECK_DOUBLE(double(node.attribute(STR("attr1")).as_float()), 0);
  CHECK_DOUBLE(double(node.attribute(STR("attr2")).as_float()), 1);
  CHECK_DOUBLE(double(node.attribute(STR("attr3")).as_float()), 0.12);
  CHECK_DOUBLE(double(node.attribute(STR("attr4")).as_float()), -5.1);
  CHECK_DOUBLE(double(node.attribute(STR("attr5")).as_float()), 3e-4);
  CHECK_DOUBLE(double(node.attribute(STR("attr6")).as_float()),
               3.14159265358979323846);
}

TEST_XML(dom_attr_as_double,
         "<node attr1='0' attr2='1' attr3='0.12' attr4='-5.1' attr5='3e-4' "
         "attr6='3.14159265358979323846'/>") {
  xml_node node = doc.child(STR("node"));

  CHECK(xml_attribute().as_double() == 0);
  CHECK_DOUBLE(node.attribute(STR("attr1")).as_double(), 0);
  CHECK_DOUBLE(node.attribute(STR("attr2")).as_double(), 1);
  CHECK_DOUBLE(node.attribute(STR("attr3")).as_double(), 0.12);
  CHECK_DOUBLE(node.attribute(STR("attr4")).as_double(), -5.1);
  CHECK_DOUBLE(node.attribute(STR("attr5")).as_double(), 3e-4);
  CHECK_DOUBLE(node.attribute(STR("attr6")).as_double(),
               3.14159265358979323846);
}

TEST_XML(dom_attr_as_bool,
         "<node attr1='0' attr2='1' attr3='true' attr4='True' attr5='Yes' "
         "attr6='yes' attr7='false'/>") {
  xml_node node = doc.child(STR("node"));

  CHECK(!xml_attribute().as_bool());
  CHECK(!node.attribute(STR("attr1")).as_bool());
  CHECK(node.attribute(STR("attr2")).as_bool());
  CHECK(node.attribute(STR("attr3")).as_bool());
  CHECK(node.attribute(STR("attr4")).as_bool());
  CHECK(node.attribute(STR("attr5")).as_bool());
  CHECK(node.attribute(STR("attr6")).as_bool());
  CHECK(!node.attribute(STR("attr7")).as_bool());
}

#ifdef PUGIXML_HAS_LONG_LONG
TEST_XML(dom_attr_as_llong,
         "<node attr1='1' attr2='-1' attr3='-9223372036854775808' "
         "attr4='9223372036854775807' attr5='0'/>") {
  xml_node node = doc.child(STR("node"));

  CHECK(xml_attribute().as_llong() == 0);
  CHECK(node.attribute(STR("attr1")).as_llong() == 1);
  CHECK(node.attribute(STR("attr2")).as_llong() == -1);
  CHECK(node.attribute(STR("attr3")).as_llong() == -9223372036854775807ll - 1);
  CHECK(node.attribute(STR("attr4")).as_llong() == 9223372036854775807ll);
  CHECK(node.attribute(STR("attr5")).as_llong() == 0);
}

TEST_XML(dom_attr_as_llong_hex,
         "<node attr1='0777' attr2='0x5ab' attr3='0XFf' attr4='-0x20' "
         "attr5='-0x8000000000000000' attr6='0x'/>") {
  xml_node node = doc.child(STR("node"));

  CHECK(node.attribute(STR("attr1")).as_llong() ==
        777);  // no octal support! intentional
  CHECK(node.attribute(STR("attr2")).as_llong() == 1451);
  CHECK(node.attribute(STR("attr3")).as_llong() == 255);
  CHECK(node.attribute(STR("attr4")).as_llong() == -32);
  CHECK(node.attribute(STR("attr5")).as_llong() == -9223372036854775807ll - 1);
  CHECK(node.attribute(STR("attr6")).as_llong() == 0);
}

TEST_XML(dom_attr_as_ullong,
         "<node attr1='0' attr2='1' attr3='9223372036854775807' "
         "attr4='18446744073709551615' attr5='0'/>") {
  xml_node node = doc.child(STR("node"));

  CHECK(xml_attribute().as_ullong() == 0);
  CHECK(node.attribute(STR("attr1")).as_ullong() == 0);
  CHECK(node.attribute(STR("attr2")).as_ullong() == 1);
  CHECK(node.attribute(STR("attr3")).as_ullong() == 9223372036854775807ull);
  CHECK(node.attribute(STR("attr4")).as_ullong() == 18446744073709551615ull);
  CHECK(node.attribute(STR("attr5")).as_ullong() == 0);
}

TEST_XML(dom_attr_as_ullong_hex,
         "<node attr1='0777' attr2='0x5ab' attr3='0XFf' attr4='0x20' "
         "attr5='0xFFFFFFFFFFFFFFFF' attr6='0x'/>") {
  xml_node node = doc.child(STR("node"));

  CHECK(node.attribute(STR("attr1")).as_ullong() ==
        777);  // no octal support! intentional
  CHECK(node.attribute(STR("attr2")).as_ullong() == 1451);
  CHECK(node.attribute(STR("attr3")).as_ullong() == 255);
  CHECK(node.attribute(STR("attr4")).as_ullong() == 32);
  CHECK(node.attribute(STR("attr5")).as_ullong() == 18446744073709551615ull);
  CHECK(node.attribute(STR("attr6")).as_ullong() == 0);
}
#endif

TEST(dom_attr_defaults) {
  xml_attribute attr;

  CHECK_STRING(attr.as_string(STR("foo")), STR("foo"));
  CHECK(attr.as_int(42) == 42);
  CHECK(attr.as_uint(42) == 42);
  CHECK(attr.as_double(42) == 42);
  CHECK(attr.as_float(42) == 42);
  CHECK(attr.as_bool(true) == true);

#ifdef PUGIXML_HAS_LONG_LONG
  CHECK(attr.as_llong(42) == 42);
  CHECK(attr.as_ullong(42) == 42);
#endif
}

TEST_XML(dom_node_bool_ops, "<node/>") {
  generic_bool_ops_test(doc.child(STR("node")));
}

TEST_XML(dom_node_eq_ops, "<node><node1/><node2/></node>") {
  generic_eq_ops_test(doc.child(STR("node")).child(STR("node1")),
                      doc.child(STR("node")).child(STR("node2")));
}

TEST_XML(dom_node_empty, "<node/>") {
  generic_empty_test(doc.child(STR("node")));
}

TEST_XML(dom_node_parent, "<node><child/></node>") {
  CHECK(xml_node().parent() == xml_node());
  CHECK(doc.child(STR("node")).child(STR("child")).parent() ==
        doc.child(STR("node")));
  CHECK(doc.child(STR("node")).parent() == doc);
}

TEST_XML(dom_node_root, "<node><child/></node>") {
  CHECK(xml_node().root() == xml_node());
  CHECK(doc.child(STR("node")).child(STR("child")).root() == doc);
  CHECK(doc.child(STR("node")).root() == doc);
}

TEST_XML(dom_node_child, "<node><child1/><child2/></node>") {
  CHECK(xml_node().child(STR("n")) == xml_node());

  CHECK(doc.child(STR("n")) == xml_node());
  CHECK_NAME_VALUE(doc.child(STR("node")), STR("node"), STR(""));
  CHECK(doc.child(STR("node")).child(STR("child2")) ==
        doc.child(STR("node")).last_child());
}

TEST_XML(dom_node_attribute, "<node attr1='0' attr2='1'/>") {
  CHECK(xml_node().attribute(STR("a")) == xml_attribute());

  xml_node node = doc.child(STR("node"));

  CHECK(node.attribute(STR("n")) == xml_attribute());
  CHECK_NAME_VALUE(node.attribute(STR("attr1")), STR("attr1"), STR("0"));
  CHECK(node.attribute(STR("attr2")) == node.last_attribute());
}

TEST_XML(dom_node_next_previous_sibling,
         "<node><child1/><child2/><child3/></node>") {
  CHECK(xml_node().next_sibling() == xml_node());
  CHECK(xml_node().next_sibling(STR("n")) == xml_node());

  CHECK(xml_node().previous_sibling() == xml_node());
  CHECK(xml_node().previous_sibling(STR("n")) == xml_node());

  xml_node child1 = doc.child(STR("node")).child(STR("child1"));
  xml_node child2 = doc.child(STR("node")).child(STR("child2"));
  xml_node child3 = doc.child(STR("node")).child(STR("child3"));

  CHECK(child1.next_sibling() == child2);
  CHECK(child3.next_sibling() == xml_node());

  CHECK(child1.previous_sibling() == xml_node());
  CHECK(child3.previous_sibling() == child2);

  CHECK(child1.next_sibling(STR("child3")) == child3);
  CHECK(child1.next_sibling(STR("child")) == xml_node());

  CHECK(child3.previous_sibling(STR("child1")) == child1);
  CHECK(child3.previous_sibling(STR("child")) == xml_node());
}

TEST_XML(dom_node_child_value,
         "<node><novalue/><child1>value1</child1><child2>value2<n/></"
         "child2><child3><![CDATA[value3]]></child3>value4</node>") {
  CHECK_STRING(xml_node().child_value(), STR(""));
  CHECK_STRING(xml_node().child_value(STR("n")), STR(""));

  xml_node node = doc.child(STR("node"));

  CHECK_STRING(node.child_value(), STR("value4"));
  CHECK_STRING(node.child(STR("child1")).child_value(), STR("value1"));
  CHECK_STRING(node.child(STR("child2")).child_value(), STR("value2"));
  CHECK_STRING(node.child(STR("child3")).child_value(), STR("value3"));
  CHECK_STRING(node.child_value(STR("child3")), STR("value3"));
  CHECK_STRING(node.child_value(STR("novalue")), STR(""));
}

TEST_XML(dom_node_first_last_attribute, "<node attr1='0' attr2='1'/>") {
  xml_node node = doc.child(STR("node"));

  CHECK(node.first_attribute() == node.attribute(STR("attr1")));
  CHECK(node.last_attribute() == node.attribute(STR("attr2")));

  CHECK(xml_node().first_attribute() == xml_attribute());
  CHECK(xml_node().last_attribute() == xml_attribute());

  CHECK(doc.first_attribute() == xml_attribute());
  CHECK(doc.last_attribute() == xml_attribute());
}

TEST_XML(dom_node_first_last_child, "<node><child1/><child2/></node>") {
  xml_node node = doc.child(STR("node"));

  CHECK(node.first_child() == node.child(STR("child1")));
  CHECK(node.last_child() == node.child(STR("child2")));

  CHECK(xml_node().first_child() == xml_node());
  CHECK(xml_node().last_child() == xml_node());

  CHECK(doc.first_child() == node);
  CHECK(doc.last_child() == node);
}

TEST_XML(dom_node_find_child_by_attribute,
         "<node><stub attr='value3' /><child1 attr='value1'/><child2 "
         "attr='value2'/><child2 attr='value3'/></node>") {
  CHECK(xml_node().find_child_by_attribute(STR("name"), STR("attr"),
                                           STR("value")) == xml_node());
  CHECK(xml_node().find_child_by_attribute(STR("attr"), STR("value")) ==
        xml_node());

  xml_node node = doc.child(STR("node"));

  CHECK(node.find_child_by_attribute(STR("child2"), STR("attr"),
                                     STR("value3")) == node.last_child());
  CHECK(node.find_child_by_attribute(STR("child2"), STR("attr3"),
                                     STR("value3")) == xml_node());
  CHECK(node.find_child_by_attribute(STR("attr"), STR("value2")) ==
        node.child(STR("child2")));
  CHECK(node.find_child_by_attribute(STR("attr3"), STR("value")) == xml_node());
}

TEST(dom_node_find_child_by_attribute_null) {
  xml_document doc;
  xml_node node0 = doc.append_child();
  xml_node node1 = doc.append_child(STR("a"));
  xml_node node2 = doc.append_child(STR("a"));
  xml_node node3 = doc.append_child(STR("a"));

  (void)node0;

  // this adds an attribute with null name and/or value in the internal
  // representation
  node1.append_attribute(STR(""));
  node2.append_attribute(STR("id"));
  node3.append_attribute(STR("id")) = STR("1");

  // make sure find_child_by_attribute works if name/value is null
  CHECK(doc.find_child_by_attribute(STR("unknown"), STR("wrong")) ==
        xml_node());
  CHECK(doc.find_child_by_attribute(STR("id"), STR("wrong")) == xml_node());
  CHECK(doc.find_child_by_attribute(STR("id"), STR("")) == node2);
  CHECK(doc.find_child_by_attribute(STR("id"), STR("1")) == node3);

  CHECK(doc.find_child_by_attribute(STR("a"), STR("unknown"), STR("wrong")) ==
        xml_node());
  CHECK(doc.find_child_by_attribute(STR("a"), STR("id"), STR("wrong")) ==
        xml_node());
  CHECK(doc.find_child_by_attribute(STR("a"), STR("id"), STR("")) == node2);
  CHECK(doc.find_child_by_attribute(STR("a"), STR("id"), STR("1")) == node3);
}

struct find_predicate_const {
  bool result;

  find_predicate_const(bool result_) : result(result_) {}

  template <typename T>
  bool operator()(const T&) const {
    return result;
  }
};

struct find_predicate_prefix {
  const char_t* prefix;

  find_predicate_prefix(const char_t* prefix_) : prefix(prefix_) {}

  template <typename T>
  bool operator()(const T& obj) const {
#ifdef PUGIXML_WCHAR_MODE
    // can't use wcsncmp here because of a bug in DMC
    return std::basic_string<char_t>(obj.name())
               .compare(0, wcslen(prefix), prefix) == 0;
#else
    return strncmp(obj.name(), prefix, strlen(prefix)) == 0;
#endif
  }
};

TEST_XML(dom_node_first_element_by_path,
         "<node><child1>text<child2/></child1></node>") {
  CHECK(xml_node().first_element_by_path(STR("/")) == xml_node());
  CHECK(xml_node().first_element_by_path(STR("a")) == xml_node());

  CHECK(doc.first_element_by_path(STR("")) == doc);
  CHECK(doc.first_element_by_path(STR("/")) == doc);

  CHECK(doc.first_element_by_path(STR("/node/")) == doc.child(STR("node")));
  CHECK(doc.first_element_by_path(STR("node/")) == doc.child(STR("node")));
  CHECK(doc.first_element_by_path(STR("node")) == doc.child(STR("node")));
  CHECK(doc.first_element_by_path(STR("/node")) == doc.child(STR("node")));

  CHECK(doc.first_element_by_path(STR("/node/child2")) == xml_node());

  CHECK(doc.first_element_by_path(STR("\\node\\child1"), '\\') ==
        doc.child(STR("node")).child(STR("child1")));

  CHECK(doc.child(STR("node")).first_element_by_path(STR("..")) == doc);
  CHECK(doc.child(STR("node")).first_element_by_path(STR(".")) ==
        doc.child(STR("node")));

  CHECK(doc.child(STR("node"))
            .first_element_by_path(STR("../node/./child1/../.")) ==
        doc.child(STR("node")));

  CHECK(doc.child(STR("node")).first_element_by_path(STR("child1")) ==
        doc.child(STR("node")).child(STR("child1")));
  CHECK(doc.child(STR("node")).first_element_by_path(STR("child1/")) ==
        doc.child(STR("node")).child(STR("child1")));
  CHECK(doc.child(STR("node")).first_element_by_path(STR("child")) ==
        xml_node());
  CHECK(doc.child(STR("node")).first_element_by_path(STR("child11")) ==
        xml_node());

  CHECK(doc.first_element_by_path(STR("//node")) == doc.child(STR("node")));
}

TEST_XML(dom_offset_debug_append, "<node/>") {
  xml_node c1 = doc.first_child();
  xml_node c2 = doc.append_child(STR("node"));
  xml_node c3 = doc.append_child(node_pcdata);

  CHECK(doc.offset_debug() == 0);
  CHECK(c1.offset_debug() == 1);
  CHECK(c2.offset_debug() == -1);
  CHECK(c3.offset_debug() == -1);

  c1.set_name(STR("nodenode"));
  CHECK(c1.offset_debug() == -1);
}

TEST_XML(dom_hash_value, "<node attr='value'>value</node>") {
  xml_node node = doc.child(STR("node"));
  xml_attribute attr = node.first_attribute();
  xml_node value = node.first_child();

  CHECK(xml_node().hash_value() == 0);
  CHECK(xml_attribute().hash_value() == 0);

  CHECK(node.hash_value() != 0);
  CHECK(value.hash_value() != 0);
  CHECK(node.hash_value() != value.hash_value());

  CHECK(attr.hash_value() != 0);

  xml_node node_copy = node;
  CHECK(node_copy.hash_value() == node.hash_value());

  xml_attribute attr_copy = attr;
  CHECK(attr_copy.hash_value() == attr.hash_value());
}

TEST_XML(dom_node_attribute_hinted, "<node attr1='1' attr2='2' attr3='3' />") {
  xml_node node = doc.first_child();
  xml_attribute attr1 = node.attribute(STR("attr1"));
  xml_attribute attr2 = node.attribute(STR("attr2"));
  xml_attribute attr3 = node.attribute(STR("attr3"));

  xml_attribute hint;
  CHECK(!xml_node().attribute(STR("test"), hint) && !hint);

  CHECK(node.attribute(STR("attr2"), hint) == attr2 && hint == attr3);
  CHECK(node.attribute(STR("attr3"), hint) == attr3 && !hint);

  CHECK(node.attribute(STR("attr1"), hint) == attr1 && hint == attr2);
  CHECK(node.attribute(STR("attr2"), hint) == attr2 && hint == attr3);
  CHECK(node.attribute(STR("attr1"), hint) == attr1 && hint == attr2);
  CHECK(node.attribute(STR("attr1"), hint) == attr1 && hint == attr2);

  CHECK(!node.attribute(STR("attr"), hint) && hint == attr2);
}

TEST_XML(
    dom_as_int_overflow,
    "<node attr1='-2147483649' attr2='2147483648' attr3='-4294967296' />") {
  xml_node node = doc.child(STR("node"));

  CHECK(node.attribute(STR("attr1")).as_int() == -2147483647 - 1);
  CHECK(node.attribute(STR("attr2")).as_int() == 2147483647);
  CHECK(node.attribute(STR("attr3")).as_int() == -2147483647 - 1);
}

TEST_XML(dom_as_uint_overflow,
         "<node attr1='-1' attr2='4294967296' attr3='5294967295' "
         "attr4='21474836479' />") {
  xml_node node = doc.child(STR("node"));

  CHECK(node.attribute(STR("attr1")).as_uint() == 0);
  CHECK(node.attribute(STR("attr2")).as_uint() == 4294967295u);
  CHECK(node.attribute(STR("attr3")).as_uint() == 4294967295u);
  CHECK(node.attribute(STR("attr4")).as_uint() == 4294967295u);
}

TEST_XML(dom_as_int_hex_overflow,
         "<node attr1='-0x80000001' attr2='0x80000000' />") {
  xml_node node = doc.child(STR("node"));

  CHECK(node.attribute(STR("attr1")).as_int() == -2147483647 - 1);
  CHECK(node.attribute(STR("attr2")).as_int() == 2147483647);
}

TEST_XML(dom_as_uint_hex_overflow,
         "<node attr1='-0x1' attr2='0x100000000' attr3='0x123456789' />") {
  xml_node node = doc.child(STR("node"));

  CHECK(node.attribute(STR("attr1")).as_uint() == 0);
  CHECK(node.attribute(STR("attr2")).as_uint() == 4294967295u);
  CHECK(node.attribute(STR("attr3")).as_uint() == 4294967295u);
}

TEST_XML(dom_as_int_many_digits,
         "<node attr1='0000000000000000000000000000000000000000000000001' />") {
  xml_node node = doc.child(STR("node"));

  CHECK(node.attribute(STR("attr1")).as_int() == 1);
  CHECK(node.attribute(STR("attr1")).as_uint() == 1);
}

TEST_XML(
    dom_as_int_hex_many_digits,
    "<node attr1='0x0000000000000000000000000000000000000000000000001' />") {
  xml_node node = doc.child(STR("node"));

  CHECK(node.attribute(STR("attr1")).as_int() == 1);
  CHECK(node.attribute(STR("attr1")).as_uint() == 1);
}

#ifdef PUGIXML_HAS_LONG_LONG
TEST_XML(dom_as_llong_overflow,
         "<node attr1='-9223372036854775809' attr2='9223372036854775808' "
         "attr3='-18446744073709551616' />") {
  xml_node node = doc.child(STR("node"));

  CHECK(node.attribute(STR("attr1")).as_llong() == -9223372036854775807ll - 1);
  CHECK(node.attribute(STR("attr2")).as_llong() == 9223372036854775807ll);
  CHECK(node.attribute(STR("attr3")).as_llong() == -9223372036854775807ll - 1);
}

TEST_XML(dom_as_ullong_overflow,
         "<node attr1='-1' attr2='18446744073709551616' "
         "attr3='28446744073709551615' attr4='166020696663385964543' />") {
  xml_node node = doc.child(STR("node"));

  CHECK(node.attribute(STR("attr1")).as_ullong() == 0);
  CHECK(node.attribute(STR("attr2")).as_ullong() == 18446744073709551615ull);
  CHECK(node.attribute(STR("attr3")).as_ullong() == 18446744073709551615ull);
  CHECK(node.attribute(STR("attr4")).as_ullong() == 18446744073709551615ull);
}

TEST_XML(dom_as_llong_hex_overflow,
         "<node attr1='-0x8000000000000001' attr2='0x8000000000000000' />") {
  xml_node node = doc.child(STR("node"));

  CHECK(node.attribute(STR("attr1")).as_llong() == -9223372036854775807ll - 1);
  CHECK(node.attribute(STR("attr2")).as_llong() == 9223372036854775807ll);
}

TEST_XML(dom_as_ullong_hex_overflow,
         "<node attr1='-0x1' attr2='0x10000000000000000' "
         "attr3='0x12345678923456789' />") {
  xml_node node = doc.child(STR("node"));

  CHECK(node.attribute(STR("attr1")).as_ullong() == 0);
  CHECK(node.attribute(STR("attr2")).as_ullong() == 18446744073709551615ull);
  CHECK(node.attribute(STR("attr3")).as_ullong() == 18446744073709551615ull);
}

TEST_XML(dom_as_llong_many_digits,
         "<node attr1='0000000000000000000000000000000000000000000000001' />") {
  xml_node node = doc.child(STR("node"));

  CHECK(node.attribute(STR("attr1")).as_llong() == 1);
  CHECK(node.attribute(STR("attr1")).as_ullong() == 1);
}

TEST_XML(
    dom_as_llong_hex_many_digits,
    "<node attr1='0x0000000000000000000000000000000000000000000000001' />") {
  xml_node node = doc.child(STR("node"));

  CHECK(node.attribute(STR("attr1")).as_llong() == 1);
  CHECK(node.attribute(STR("attr1")).as_ullong() == 1);
}
#endif

TEST_XML(dom_as_int_plus, "<node attr1='+1' attr2='+0xa' />") {
  xml_node node = doc.child(STR("node"));

  CHECK(node.attribute(STR("attr1")).as_int() == 1);
  CHECK(node.attribute(STR("attr1")).as_uint() == 1);
  CHECK(node.attribute(STR("attr2")).as_int() == 10);
  CHECK(node.attribute(STR("attr2")).as_uint() == 10);

#ifdef PUGIXML_HAS_LONG_LONG
  CHECK(node.attribute(STR("attr1")).as_llong() == 1);
  CHECK(node.attribute(STR("attr1")).as_ullong() == 1);
  CHECK(node.attribute(STR("attr2")).as_llong() == 10);
  CHECK(node.attribute(STR("attr2")).as_ullong() == 10);
#endif
}

TEST(dom_node_anonymous) {
  xml_document doc;
  doc.append_child(node_element);
  doc.append_child(node_element);
  doc.append_child(node_pcdata);

  CHECK(doc.child(STR("node")) == xml_node());
  CHECK(doc.first_child().next_sibling(STR("node")) == xml_node());
  CHECK(doc.last_child().previous_sibling(STR("node")) == xml_node());
  CHECK_STRING(doc.child_value(), STR(""));
  CHECK_STRING(doc.last_child().child_value(), STR(""));
}
