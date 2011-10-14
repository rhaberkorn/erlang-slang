_debug_info = 1; () = evalfile ("inc.sl");

print ("Testing Arithmetic ...");

static variable G = 11;
define check_global_local ()
{
   variable x = 1;
   if (G + 1 != 12)
     failed ("global + int");
   if (1 + G != 12)
     failed ("int + global");
   if (x + 11 != 12)
     failed ("local + int");
   if (11 + x != 12)
     failed ("int + local");
   if (x + x != 2)
     failed ("local + local");
   if (x + G != 12)
     failed ("local + global");
   if (G + x != 12)
     failed ("global + local");
   if (1 + 11 != 12)
     failed ("int + int");
}
check_global_local ();

define check_typeof (expr, type)
{
   if (typeof (expr) != type)
     failed ("typeof " + string (type) + " found " + string (typeof(expr)));
}

define check_bool (i)
{
   check_typeof (i == i, Char_Type);
}

define check_sum_result (i, j, k)
{
   if (k != i + j)
     failed (sprintf("%S + %S != %S", typeof (i), typeof(j), typeof(k)));
}

check_typeof('a', UChar_Type);
check_typeof(1h, Short_Type);
check_typeof(1hu, UShort_Type);
check_typeof(0x20hu, UShort_Type);
check_typeof(1, Integer_Type);
check_typeof(0x20, Integer_Type);
check_typeof(1u, UInteger_Type);
check_typeof(1LU, ULong_Type);
#ifexists Double_Type
check_typeof(1f, Float_Type);
check_typeof(1e10f, Float_Type);
check_typeof(.1e10f, Float_Type);
check_typeof(.1e10, Double_Type);
#endif
check_typeof(~'a', UChar_Type);
check_typeof(~1h, Short_Type);
check_typeof(~1hu, UShort_Type);
check_typeof(~0x20hu, UShort_Type);
check_typeof(~1, Integer_Type);
check_typeof(~0x20, Integer_Type);
check_typeof(~1u, UInteger_Type);
check_typeof(~1LU, ULong_Type);

check_typeof ('a' + 'b', Integer_Type);
check_typeof (1h + 'b', Integer_Type);

if (Integer_Type == Short_Type) check_typeof (1hu + 'b', UInteger_Type);
else check_typeof (1hu + 'b', Integer_Type);

check_typeof (1u + 1, UInteger_Type);

if (Integer_Type == Long_Type) check_typeof (1u + 1L, ULong_Type);
else  check_typeof (1u + 1L, Long_Type);

check_typeof (1u + 1UL, ULong_Type);
#ifexists Double_Type
check_typeof (1u + 1.0f, Float_Type);
check_typeof (1u + 1.0, Double_Type);
#endif
#ifexists Complex_Type
check_typeof ('c' * 1i, Complex_Type);
check_typeof (1h * 1i, Complex_Type);
check_typeof (1.0 * 1i, Complex_Type);
check_typeof (1i * 1i, Complex_Type);
#endif

check_bool ('a');
check_bool (1h);
check_bool (1hu);
check_bool (1);
check_bool (1u);
check_bool (1L);
check_bool (1LU);
#ifexists Double_Type
check_bool (1f);
check_bool (1.0);
#endif
#ifexists Complex_Type
check_bool (1.0i);
#endif

#ifexists Complex_Type
check_typeof (Real(1), Double_Type);
check_typeof (Real('a'), Double_Type);
check_typeof (Real(1L), Double_Type);
check_typeof (Real(1f), Float_Type);
check_typeof (Real(1.0), Double_Type);
#endif

check_sum_result (1, 1, 2);
check_sum_result (1, 0x31, 50);
check_sum_result (1, '1', 50);
check_sum_result (1L, '1', 50L);
check_sum_result (1L, 1h, 2L);
check_sum_result (1, 1h, 2);
check_sum_result (1h, '1', 50);
check_sum_result (1u, 3, 4);
check_sum_result (1UL, '\x3', 4UL);

#ifexists Complex_Type
static define check_complex_fun (fun, x)
{
   variable z = x + 0i;
   variable diff = abs (@fun(z) - @fun(x));
   if (diff > 1e-13)
     failed ("%S %S", fun, z);
}

check_complex_fun (&sin, 1);
check_complex_fun (&cos, 1);
check_complex_fun (&tan, 1);
check_complex_fun (&acos, 0.5);
check_complex_fun (&asin, 0.5);
check_complex_fun (&atan, 0.5);
check_complex_fun (&cosh, 1);
check_complex_fun (&sinh, 1);
check_complex_fun (&tanh, 1);
check_complex_fun (&asinh, 0.5);
check_complex_fun (&acosh, 2.0);
check_complex_fun (&atanh, 0.5);
check_complex_fun (&sqrt, 0.5);
check_complex_fun (&exp, 0.5);
#endif

define test_eqs (a, b, r)
{
   if (r != __eqs (a,b))
     failed ("__eqs (%S,%S)", a, b);
}

test_eqs (1,1,1);
test_eqs (1,'\001', 0);
#ifexists Double_Type
test_eqs (1, 1.0, 0);
#endif
test_eqs ("xyz", "xyz", 1);
test_eqs ([1:3],[1:3],0);
test_eqs (stdout, stderr, 0);
test_eqs (stderr, 1, 0);
#ifexists Complex_Type
test_eqs (1+2i, 1+2i, 1);
test_eqs (1.0+0.0i, 1.0, 0);
#endif

#ifexists Double_Type
define another_test ()
{
   variable x = 1.0;
   variable y;

   if (18 != 1.0+1+x
       + 1.0+x+1
       + x+1.0+1
       + x+1+1.0
       + 1+1.0+x
       + 1+x+1.0)
     failed ("sum combinations");
}

another_test();
#endif

define test_typecast ()
{
   variable args = __pop_args (_NARGS-1);
   variable y = ();
   
   if (y != typecast (__push_args (args)))
     failed ("typecast");
}

#ifexists Double_Type
test_typecast (0.0f, 0, Float_Type);
#endif

print ("Ok\n");
exit (0);
