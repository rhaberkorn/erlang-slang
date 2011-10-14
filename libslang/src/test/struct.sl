_debug_info = 1; () = evalfile ("inc.sl");

print ("Testing structures ...");

variable S = struct 
{
   a, b, c
};

S.a = "a";
S.b = "b";
S.c = "c";

variable U = @Struct_Type ("a", "b", "c");
variable abc = get_struct_field_names (U);
if ((abc[0] != "a")
    or (abc[1] != "b")
    or (abc[2] != "c"))
  failed ("@Struct_Type");

abc = ["a", "b", "c"];
U = @Struct_Type (abc);
if (length (where (abc != get_struct_field_names (U))))
  failed ("@Struct_Type([abc])");

variable T = @S;

if (S.a != T.a) failed ("Unable to copy via @S");
if (S.b != T.b) failed ("Unable to copy via @S");
if (S.c != T.c) failed ("Unable to copy via @S");

T.a = "XXX";
if (T.a == S.a) failed ("Unable to copy via @S");

set_struct_fields (T, 1, 2, "three");
if ((T.c != "three") or (T.a != 1) or (T.b != 2))
  failed ("set_struct_fields");

T.a++;
T.a += 3;
T.a -= 20;
if (T.a != -15) 
  failed ("structure arithmetic");

T.c = S;
S.a = T;

if (T != T.c.a)  
  failed ("Unable to create a circular list");

typedef struct 
{
   TT_x, TT_y
}
TT;

T = @TT;
if (typeof (T) != TT)
  failed ("typeof(T)");
if (0 == is_struct_type (T))
  failed ("is_struct_type");
S = typecast (T, Struct_Type);
if (typeof (S) != Struct_Type)
  failed ("typecast");

% C structures

S = get_c_struct ();
if ((typeof (S.h) != Short_Type)
    or (typeof (S.l) != Long_Type)
    or (typeof (S.b) != Char_Type))
  failed ("get_c_struct field types");

static define print_struct(s)
{
   foreach (get_struct_field_names (s))
     {
	variable f = ();
	vmessage ("S.%s = %S", f, get_struct_field (s, f));
     }
}


#ifexists Complex_Type
S.z = 1+2i;
#endif
S.a = [1:10];
#ifexists Double_Type
S.d = PI;
#endif
S.s = "foobar";
S.ro_str = "FOO";

loop (10)
  set_c_struct (S);

loop (10)
  T = get_c_struct ();

%print_struct (T);

if ((not __eqs(S.a, T.a))
#ifexists Complex_Type
    or (S.z != T.z)
#endif
#ifexists Double_Type
    or (S.d != T.d)
#endif
    or (T.ro_str != "read-only"))
  failed ("C Struct");

loop (10)
  get_c_struct_via_ref (&T);

%print_struct (T);

if ((not __eqs(S.a, T.a))
#ifexists Complex_Type
    or (S.z != T.z)
#endif
#ifexists Double_Type
    or (S.d != T.d)
#endif
    or (T.ro_str != "read-only"))
  failed ("C Struct");

static define count_args ()
{
   if (_NARGS != 0)
     failed ("foreach using with NULL");
}
static define test_foreach_using_with_null (s)
{
   foreach (s) using ("next")
     {
	s = ();
     }
   count_args ();
}
test_foreach_using_with_null (NULL);
print ("Ok\n");

exit (0);

