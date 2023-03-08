%module example

%{
#include "models/polymorphic_types.h"
#include "models/aggregate_struct.h"
#include "models/derived_agregate_types.h"
#include "models/derived_class_types.h"
#include "models/final_class.h"
#include "models/free_functions.h"
#include "models/enum.h"
#include "models/crossfile_types.h"
%}
%include "models/final_class.h"
%include "models/free_functions.h"
%include "models/aggregate_struct.h"
%include "models/enum.h"
%include "models/polymorphic_types.h"
%include "models/derived_agregate_types.h"
%include "models/derived_class_types.h"
%include "models/crossfile_types.h"
