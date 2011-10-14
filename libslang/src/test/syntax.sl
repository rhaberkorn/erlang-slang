_debug_info = 1; () = evalfile ("inc.sl");

print ("Testing syntax ...");

if (0x12 != test_char_return (0x12)) failed ("test_char_return");
if (0x1234h != test_short_return (0x1234h)) failed ("test_short_return");
if (0x1234 != test_int_return (0x1234)) failed ("test_int_return");
if (0x12345678L != test_long_return (0x12345678L)) failed ("test_long_return");
% if (1.2e34f != test_float_return (1.2e34f)) failed ("test_float_return");
#ifexists Double_Type
if (1.2e34 != test_double_return (1.2e34)) failed ("test_double_return");
#endif

static define static_xxx ()
{
   return "xxx";
}

private define private_yyy ()
{
   return "yyy";
}

public define public_zzz ()
{
   return "zzz";
}

if (is_defined ("static_xxx") or "xxx" != static_xxx ())
  failed ("static_xxx");
if (is_defined ("private_yyy") or "yyy" != private_yyy ())
  failed ("private_yyy");
if (not is_defined ("public_zzz") or "zzz" != public_zzz ())
  failed ("public_xxx");

variable XXX = 1;
static define xxx ()
{
   variable XXX = 2;
   if (XXX != 2) failed ("local variable XXX");
}

xxx ();
if (XXX != 1) failed ("global variable XXX");
if (1)
{
   if (orelse
	{0}
	{0}
	{0}
	{0}
       ) 
     failed ("orelse");
}


!if (orelse
     {0}
     {0}
     {0}
     {1}) failed ("not orelse");

_auto_declare = 1;
XXX_auto_declared = 1;

if (&XXX_auto_declared != __get_reference ("XXX_auto_declared"))
  failed ("__get_reference");

if (0 == __is_initialized (&XXX_auto_declared))
  failed ("__is_initialized");
() = __tmp (XXX_auto_declared);
if (__is_initialized (&XXX_auto_declared))
  failed ("__is_initialized __tmp");
XXX_auto_declared = "xxx";
__uninitialize (&XXX_auto_declared);
if (__is_initialized (&XXX_auto_declared))
  failed ("__is_initialized __uninitialize");

static define test_uninitialize ()
{
   variable x;
   if (__is_initialized (&x))
     failed ("__is_initialized x");
   x = 3;
   !if (__is_initialized (&x))
     failed ("__is_initialized x=3");
   if (3 != __tmp (x))
     failed ("__tmp return value");
   if (__is_initialized (&x))
     failed ("__tmp x");
   x = 4;
   __uninitialize (&x);
   if (__is_initialized (&x))
     failed ("__uninitialize x");
}

test_uninitialize ();

static define check_args (n)
{
   if (n + 1 != _NARGS)
     failed ("check_args %d", n);
   _pop_n (_NARGS-1);
}

static define nitems (n)
{
   loop (n) 1;
}

check_args (1, 1);
check_args (1,2,2);
check_args (nitems(3), nitems(5), 8);
static variable X = [1:10];
% X[3]++ produces nothing
check_args (nitems (3), check_args(nitems(4), X[3]++, 4, X[3]+=X[2], 5), 3);
   
static define check_no_args ()
{
   if (_NARGS != 0)
     failed ("check_no_args");
}

% This failed in previous versions because abs was not treated as a function 
% call.
if (abs (1) > 0)
  check_no_args ();

define check_tmp_optim ()
{
   variable a = [1:10:1.0];
   variable b = a*0.0;
   if ((a[0] != 1.0) or (__eqs(a,b)))
     failed ("__tmp optimization: a[0] = %f", a[0]);
}

check_tmp_optim ();

print ("Ok\n");

exit (0);

