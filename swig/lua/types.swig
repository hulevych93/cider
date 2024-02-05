%{
bool is_typed(lua_State *L, int idx, swig_type_info* desc) {
  void* ptr = nullptr;
  return SWIG_CheckState(SWIG_ConvertPtr(L, idx, (void**)&ptr, desc, 0)) && ptr != nullptr;
}
%}

%define prv_decl_typecheck_typemap(TYPE_CHECKER, PR, TYPE)
  %typemap(typecheck, precedence=PR) TYPE, TYPE&, TYPE*
  %{
    $1 = TYPE_CHECKER($input, TYPE);
  %}
%enddef

%define prv_decl_in_typemap(TYPE)
  %typemap(in) TYPE
  {
  cider::TypedValue<TYPE>* val_;
  void* argp = nullptr;
  if (!SWIG_IsOK(SWIG_ConvertPtr(L, $input, &argp, $descriptor(cider::TypedValue<TYPE>*), 0)))
  {
    SWIG_fail_arg("$symname", $argnum, #TYPE);
  }
  if (!argp)
  {
    SWIG_fail_ptr("$symname", $argnum, $descriptor(cider::TypedValue<TYPE>*));
  }
  else
  {
    val_ = reinterpret_cast<cider::TypedValue<TYPE>*>(argp);
    $1 = val_->value;
  }
  }
%enddef

%define prv_is_typed(SOURCE, TYPE)
  is_typed(L, SOURCE, $descriptor(cider::TypedValue<TYPE>*));
%enddef

%define prv_is_float(SOURCE, TYPE)
  lua_isnumber(L, SOURCE)
%enddef

%define prv_is_string(SOURCE, TYPE)
  (lua_isstring(L, SOURCE) || lua_type(L, SOURCE) == LUA_TSTRING)
%enddef

%define prv_is_bool(SOURCE, TYPE)
  lua_isboolean(L, SOURCE)
%enddef

%define decl_integer_based(TYPE)
  prv_decl_typecheck_typemap(prv_is_typed, SWIG_TYPECHECK_INT32, TYPE)
  prv_decl_in_typemap(TYPE)
%enddef

%define decl_enum_based(TYPE)
  prv_decl_typecheck_typemap(prv_is_typed, SWIG_TYPECHECK_COMPLEX, TYPE)
  prv_decl_in_typemap(TYPE)
%enddef

%define decl_float_based(TYPE)
  prv_decl_typecheck_typemap(prv_is_float, SWIG_TYPECHECK_FLOAT, TYPE)
%enddef

%define decl_float_enum(TYPE)
  prv_decl_typecheck_typemap(prv_is_float, SWIG_TYPECHECK_FLOAT, TYPE)
%enddef

%define decl_string_based(TYPE)
  prv_decl_typecheck_typemap(prv_is_string, SWIG_TYPECHECK_STRING, TYPE)
%enddef

%define decl_bool_based(TYPE)
  prv_decl_typecheck_typemap(prv_is_bool, SWIG_TYPECHECK_BOOL, TYPE)
%enddef
