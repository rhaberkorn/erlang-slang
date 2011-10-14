
_debug_info = 1; () = evalfile ("inc.sl");

print ("Testing array functions ...");

static variable A = [0:23];

static variable B = transpose(A);
static variable dims;

(dims,,) = array_info (B);
if ((dims[0] != 1)
    or (dims[1] != 24))
  failed ("transpose ([0:23])");


reshape (A, [2,3,4]);

static define eqs (a, b)
{
   variable len;
   len = length (a);
   if (len != length (b))
     return 0;
   
   len == length (where (a == b));
}

static define neqs (a, b)
{
   not (eqs (a, b));
}


if ((A[0,0,0] != 0)
    or (A[0,0,1] != 1)
    or (neqs (A[0,0,[:]], [0:3]))
    or (neqs (A[0,1,[:]], [4:7]))
    or (neqs (A[0,2,[:]], [8:11]))
    or (neqs (A[1,0,[:]], [12:15]))
    or (neqs (A[1,1,[:]], [16:19]))
    or (neqs (A[1,2,[:]], [20:23]))) failed ("reshape");

B = transpose (A);

if ((B[0,0,0] != 0)
    or (B[1,0,0] != 1)
    or (neqs (B[[:],0,0], [0:3]))
    or (neqs (B[[:],1,0], [4:7]))
    or (neqs (B[[:],2,0], [8:11]))
    or (neqs (B[[:],0,1], [12:15]))
    or (neqs (B[[:],1,1], [16:19]))
    or (neqs (B[[:],2,1], [20:23]))) failed ("transpose int array");

% Test for memory leak
loop (100) B = transpose (B);
B = 0;

% Try on a string array
variable S = String_Type[length (A)];
foreach (A)
{
   variable i = ();
   S[i] = string (i);
}

variable T = @S;
reshape (S, [2,3,4]);

if ((S[0,0,0] != T[0])
    or (S[0,0,1] != T[1])
    or (neqs (S[0,0,*], T[[0:3]]))
    or (neqs (S[0,1,*], T[[4:7]]))
    or (neqs (S[0,2,*], T[[8:11]]))
    or (neqs (S[1,0,*], T[[12:15]]))
    or (neqs (S[1,1,*], T[[16:19]]))
    or (neqs (S[1,2,*], T[[20:23]]))) failed ("reshape string array");

S = transpose (S);

if ((S[0,0,0] != T[0])
    or (S[1,0,0] != T[1])
    or (neqs (S[*,0,0], T[[0:3]]))
    or (neqs (S[*,1,0], T[[4:7]]))
    or (neqs (S[*,2,0], T[[8:11]]))
    or (neqs (S[*,0,1], T[[12:15]]))
    or (neqs (S[*,1,1], T[[16:19]]))
    or (neqs (S[*,2,1], T[[20:23]]))) failed ("transpose string array");


S = ["", "1", "12", "123", "1234", "12345"];
S = array_map (Int_Type, &strlen, S);
if (neqs (S, [0:5])) failed ("array_map 1");

S = ["", "1", "12", "123", "1234", "12345"];
variable SS = S + S;
if (neqs (SS, array_map (String_Type, &strcat, S, S))) failed ("array_map 2");

SS = S + "--end";
if (neqs (SS, array_map (String_Type, &strcat, S, "--end"))) failed ("array_map 3");

#ifexists Double_Type
S = [1:20:0.1];
if (neqs (sin(S), array_map (Double_Type, &sin, S))) failed ("array_map 3");

S = [1:20:0.1];
variable Sin_S = Double_Type[length(S)];
static define void_sin (x, i)
{
   Sin_S[i] = sin (x);
}
array_map (Void_Type, &void_sin, S, [0:length(S)-1]);
if (neqs (sin(S), Sin_S))
  failed ("array_map Void_Type");
#endif

% Check indexing with negative subscripts 
S = [0:10];

if (S[-1] != 10) failed ("[-1]");
if (length (S[[-1:3]])) failed ("[-1:3]");
if (neqs(S[[-1:0:-1]], [10:0:-1])) failed ("[-1:0:-1]");
if (neqs(S[[0:-1]], S)) failed ("[0:-1]");
if (neqs(S[[3:-1]], [3:10])) failed ([3:-1]);
if (length (S[[0:-1:-1]])) failed ("[0:-1:-1]");   %  first to last by -1
if (neqs(S[[0:]], S)) failed ("[0:]");
if (neqs(S[[:-1]], S)) failed ("[:-1]");

S = Int_Type[0];
if (length (S) != 0) failed ("Int_Type[0]");
if (neqs (S, S[[0:-1]])) failed ("Int_Type[0][[0:-1]]");


S = bstring_to_array ("hello");
if ((length (S) != 5) 
    or (typeof (S) != Array_Type)) failed ("bstring_to_array");
if ("hello" != array_to_bstring (S)) failed ("array_to_bstring");

A = ['a':'z'];
foreach (A)
{
   $1 = ();
   if (A[$1 - 'a'] != $1)
     failed ("['a':'z']");
}

define check_result (result, answer, op)
{
   if (neqs (answer, result))
     failed ("Binary operation `%s' failed", op);
}

check_result ([1,2] + [3,4], [4,6],"+");
check_result (1 + [3,4], [4,5],"+");
check_result ([3,4] + 1, [4,5],"+");

check_result ([1,2] - [3,4], [-2,-2],"-");
check_result (1 - [3,4], [-2,-3],"-");
check_result ([3,4] - 1, [2,3],"-");

check_result ([1,2] * [3,4], [3,8], "*");
check_result (1 * [3,4], [3,4], "*");
check_result ([3,4] * 1, [3,4], "*");

check_result ([12,24] / [3,4], [4,6],"/");
check_result (12 / [3,4], [4,3],"/");
check_result ([3,4] / 1, [3,4],"/");

check_result ([1,2] mod [3,4], [1,2],"mod");
check_result (3 mod [3,2], [0,1],"mod");
check_result ([3,4] mod 4, [3,0],"mod");

check_result ([1,2] == [3,2], [0,1],"==");
check_result (3 == [3,4], [1,0],"==");
check_result ([3,4] == 1, [0,0],"==");

check_result ([1,2] != [3,2], [1,0],"!=");
check_result (3 != [3,4], [0,1],"!=");
check_result ([3,4] != 1, [1,1],"!=");

check_result ([1,2] > [3,2], [0,0],">");
check_result (1 > [3,4], [0,0],">");
check_result ([3,4] > 1, [1,1],">");

check_result ([1,2] >= [3,2], [0,1],">=");
check_result (1 >= [3,4], [0,0],">=");
check_result ([3,4] >= 1, [1,1],">=");

check_result ([1,2] >= [3,2], [0,1],">=");
check_result (1 >= [3,4], [0,0],">=");
check_result ([3,4] >= 1, [1,1],">=");

check_result ([1,2] < [3,2], [1,0],"<");
check_result (1 < [3,4], [1,1],"<");
check_result ([3,4] < 1, [0,0],"<");

check_result ([1,2] <= [3,2], [1,1],"<=");
check_result (1 <= [3,4], [1,1],"<=");
check_result ([3,4] <= 1, [0,0],"<=");
#ifexists Double_Type
check_result ([1,2] ^ [3,2], [1,4],"^");
check_result (1 ^ [3,4], [1,1],"^");
check_result ([3,4] ^ 1, [3,4],"^");
check_result ([3,4] ^ 0, [1,1],"^");
#endif
check_result ([1,2] or [3,2], [1,1],"or");
check_result (1 or [3,4], [1,1],"or");
check_result ([0,1] or 1, [1,1],"or");

check_result ([1,2] and [3,2], [1,1],"and");
check_result (1 and [0,4], [0,1],"and");
check_result ([3,4] and 0, [0,0],"and");

check_result ([1,2] & [3,2], [1,2],"&");
check_result (1 & [3,4], [1,0],"&");
check_result ([3,4] & 1, [1,0],"&");

check_result ([1,2] | [3,2], [3,2],"|");
check_result (1 | [3,4], [3,5],"|");
check_result ([3,4] | 1, [3,5],"|");

check_result ([1,2] xor [3,2], [2,0],"xor");
check_result (1 xor [3,4], [2,5],"xor");
check_result ([3,4] xor 1, [2,5],"xor");

check_result ([1,2] shl [3,2], [8,8],"shl");
check_result (1 shl [3,4], [8,16],"shl");
check_result ([3,4] shl 1, [6,8],"shl");

check_result ([1,4] shr [3,1], [0,2],"shr");
check_result (8 shr [3,4], [1,0],"shr");
check_result ([3,4] shr 1, [1,2],"shr");

% Test __tmp optimizations
static define test_tmp ()
{
   variable x = [1:100];
   x = 1*__tmp(x)*1 + 1;
   if (neqs (x), [2:101])
     failed ("__tmp optimizations");
}

static define ones ()
{
   variable a;
   
   a = __pop_args (_NARGS);
   return @Array_Type (Integer_Type, [__push_args (a)]) + 1;
}

variable X = ones (5, 10);

(dims,,) = array_info (X);
if ((dims[0] != 5) or (dims[1] != 10))
  failed ("ones dims");
if (length (where (X != 1)))
  failed ("ones 1");


define test_assignments (x, i, a)
{
   variable y, z;

   y = @x; z = @x; y[i] += a; z[i] = z[i] + a; check_result (y, z, "[]+=");
   y = @x; z = @x; y[i] -= a; z[i] = z[i] - a; check_result (y, z, "[]-=");
   y = @x; z = @x; y[i] /= a; z[i] = z[i] / a; check_result (y, z, "[]/=");
   y = @x; z = @x; y[i] *= a; z[i] = z[i] * a; check_result (y, z, "[]*=");
   
   y = @x; z = @x; y[i]++; z[i] = z[i] + 1; check_result (y, z, "[]++");
   y = @x; z = @x; y[i]--; z[i] = z[i] - 1; check_result (y, z, "[]--");
}

test_assignments ([1:10], 3, 5);
test_assignments ([1:10], [3], 5);
test_assignments ([1:10], [1,3,5], 5);

% Test semi-open intervals
define test_semiopen (a, b, dx, n)
{
   variable last, first;
   variable aa = [a:b:dx];

   if (length (aa) != n)
     failed ("test_semiopen (%S,%S,%S,%S): length==>%d", a, b, dx, n, length(aa));
   
   if (n == 0)
     return;
   
   first = aa[0];
   if (first != a)
     failed ("test_semiopen (%S,%S,%S,%S): first", a, b, dx, n);
	
   last = a[-1];
   if (dx > 0)
     {
	if (last >= b)
	  failed ("test_semiopen (%S,%S,%S,%S): last", a, b, dx, n);
     }
   else if (last <= b)
	  failed ("test_semiopen (%S,%S,%S,%S): last", a, b, dx, n);
}
#ifexists Double_Type
test_semiopen (1.0, 10.0, 1.0, 9);
test_semiopen (1.0, 1.0, 12.0, 0);
test_semiopen (1.0, 1.2, -1.0, 0);
test_semiopen (1.0, 0.0, -1.0, 1);
test_semiopen (1.0, -0.0001, -1.0, 2);
#endif

A = 3; if (typeof (A[*]) != Array_Type) failed ("A[*]");

static define test_inline_array (a, type)
{
   if (_typeof (a) != type)
     failed ("test_inline_array: %S is not %S type", a, type);
}

test_inline_array ([1,2,3], Int_Type);
test_inline_array ([1L,2L,3L], Long_Type);
test_inline_array ([1h,2h,3h], Short_Type);
#ifexists Double_Type
test_inline_array ([1f, 0, 0], Float_Type);
test_inline_array ([1f, 0.0, 0h], Double_Type);
#endif
#ifexists Complex_Type
test_inline_array ([1f, 0.0, 0i], Complex_Type);
test_inline_array ([1i, 0h, 0i], Complex_Type);
test_inline_array ([0h, 0i], Complex_Type);
test_inline_array ([0i, 0i], Complex_Type);
#endif
test_inline_array (["a", "b"], String_Type);

A = String_Type[10];
A[*] = "a";
if ("aaaaaaaaaa" != strjoin (A, ""))
  failed ("A[*]");
A[5] = NULL;
if ((A[5] != NULL) 
    or ("aaaaaaaaa" != strjoin (A[[0,1,2,3,4,6,7,8,9]], "")))
  failed ("A[5] != NULL");

A[1] = NULL;
if ((length(where(_isnull(A))) != 2)
     or (where (_isnull(A))[0] != 1)
     or (where (_isnull(A))[1] != 5))
  failed ("_isnull: %S", where(_isnull(A))[1] != 5);

A[*] = "a";
if ("aaaaaaaaaa" != strjoin (A, ""))
  failed ("A[5]=a");
A[[3,7]] = NULL;
if ((A[3] != NULL) or (A[7] != NULL)
    or ("aaaaaaaa" != strjoin (A[[0,1,2,4,5,6,8,9]], "")))
  failed ("A[3,7]=NULL");

A = String_Type[10];
A[*] = "a";
A[1] = NULL;
if (length (where (A != String_Type[10])) != 9)
  failed ("A != String_Type[10]");


% Test array summing operations
#ifexists Double_Type
static define compute_sum (a, n)
{
   variable s = 0;
   variable b;
   variable i, j, k;
   variable dims;
   
   (dims,,) = array_info (a);
   if (n == 0)
     {
	b = Double_Type[dims[1],dims[2]];
	for (i = 0; i < dims[1]; i++)
	  {
	     for (j = 0; j < dims[2]; j++)
	       {
		  for (k = 0; k < dims[n]; k++)
		    b[i,j] += a[k,i,j];
	       }
	  }
	return b;
     }
   if (n == 1)
     {
	b = Double_Type[dims[0],dims[2]];
	for (i = 0; i < dims[0]; i++)
	  {
	     for (j = 0; j < dims[2]; j++)
	       {
		  for (k = 0; k < dims[n]; k++)
		    b[i,j] += a[i,k,j];
	       }
	  }
	return b;
     }
   if (n == 2)
     {
	b = Double_Type[dims[0],dims[1]];
	for (i = 0; i < dims[0]; i++)
	  {
	     for (j = 0; j < dims[1]; j++)
	       {
		  for (k = 0; k < dims[n]; k++)
		    b[i,j] += a[i,j,k];
	       }
	  }
	return b;
     }
   
   b = 0.0;
   for (i = 0; i < dims[0]; i++)
     {
	for (j = 0; j < dims[1]; j++)
	  {
	     for (k = 0; k < dims[2]; k++)
	       b += a[i,j,k];
	  }
     }
   return b;
}

A = [1:3*4*5];
reshape (A, [3,4,5]);

define test_sum (a, n)
{
   variable s1, s2;

   if (n == -1)
     s1 = sum(A);
   else
     s1 = sum(A,n);
   
   s2 = compute_sum (A, n);

   if (neqs (s1, s2))
     {
	failed ("sum(A,%d): %S != %S: %g != %g", n, s1, s2, s1[0,0], s2[0,0]);
     }
}

test_sum (A,-1);
test_sum (A,2);
test_sum (A,1);
test_sum (A,0);

A = [1+2i, 2+3i, 3+4i];
if (sum(A) != A[0] + A[1] + A[2])
  failed ("sum(Complex)");
#endif				       %  Double_Type

define find_min (a)
{
   variable m = a[0];
   _for (1, length(a)-1, 1)
     {
	variable i = ();
	if (a[i] < m)
	  m = a[i];
     }
   return m;
}

define find_max (a)
{
   variable m = a[0];
   _for (1, length(a)-1, 1)
     {
	variable i = ();
	if (a[i] > m)
	  m = a[i];
     }
   return m;
}

define test_eqs (what, a, b)
{
   if (_typeof(a) != _typeof(b))
     failed ("%s: %S != %S", what, a, b);
   
   if (neqs (a, b))
     failed ("%s: %S != %S", what, a, b);
}

A = [1:10];
test_eqs ("min", min(A), find_min(A));
test_eqs ("max", max(A), find_max(A));
#ifexists Double_Type
A *= 1.0f;
test_eqs ("min", min(A), find_min(A));
test_eqs ("max", max(A), find_max(A));
A *= 1.0;
test_eqs ("min", min(A), find_min(A));
test_eqs ("max", max(A), find_max(A));
#endif
A = [1h:10h];
test_eqs ("min", min(A), find_min(A));
test_eqs ("max", max(A), find_max(A));
A = ['0':'9'];
test_eqs ("min", min(A), find_min(A));
test_eqs ("max", max(A), find_max(A));

A=Int_Type[10,10];
A[*,*] = [0:99];
if (length (A[[0:99:11]]) != 10)
  failed ("A[[0:99:11]");

#ifexists cumsum
static define do_cumsum (a)
{
   variable b = 1.0 * a;
   variable i, s;
   
   s = 0;
   _for (0, length(a)-1, 1)
     {
	i = ();
	s += a[i];
	b[i] = s;
     }
   return b;
}
   
static define test_cumsum (a, k, result_type)
{
   variable b = 1.0 * a;
   variable bb;

   variable dims, ndims;
   variable i, j;
   (dims, ndims, ) = array_info (a);

   if (k != -1)
     bb = cumsum (a, k);
   else
     bb = cumsum (a);

   if (_typeof (bb) != result_type)
     {
	failed ("cumsum(%S) has wrong return type (%S)", a, b);
     }
#ifexists Complex_Type
   if ((_typeof (a) != Complex_Type) and (_typeof (a) != Float_Type))
#endif
     a = typecast (a, Double_Type);

   if (k == -1)
     {
	b = do_cumsum (_reshape (a, [length(a)]));
     }
   else switch (ndims)
     {
      case 1:
	b = cumsum (a);
     }
     {
      case 2:
	if (k == 0)
	  {
	     %a_j = cumsum_i a_ij
	     _for (0, dims[1]-1, 1)
	       {
		  j = ();
		  b[*, j] = do_cumsum (a[*, j]);
	       }
	  }
	else
	  {
	     _for (0, dims[1]-1, 1)
	       {
		  i = ();
		  b[i, *] = do_cumsum (a[i, *]);
	       }
	  }
     }
     {
      case 3:
	if (k == 0)
	  {
	     %a_j = cumsum_i a_ij
	     _for (0, dims[1]-1, 1)
	       {
		  i = ();
		  _for (0, dims[2]-1, 1)
		    {
		       j = ();
		       b[*, i, j] = do_cumsum (a[*, i, j]);
		    }
	       }
	  }
	else if (k == 1)
	  {
	     _for (0, dims[0]-1, 1)
	       {
		  i = ();
		  _for (0, dims[2]-1, 1)
		    {
		       j = ();
		       b[i, *, j] = do_cumsum (a[i, *, j]);
		    }
	       }
	  }
	else
	  {
	     _for (0, dims[0]-1, 1)
	       {
		  i = ();
		  _for (0, dims[1]-1, 1)
		    {
		       j = ();
		       b[i, j, *] = do_cumsum (a[i, j, *]);
		    }
	       }
	  }
     }
   
   if (neqs (b, bb))
     {
	failed ("cumsum (%S, %d), expected %S, got %S", a, k, b, bb);
     }
}


A = Int_Type[10]; A[*] = 1;
test_cumsum (A, -1, Double_Type);
test_cumsum (A, 0, Double_Type);
A = [1:3*4*5];
reshape (A, [3,4,5]);
test_cumsum (A, -1, Double_Type);
test_cumsum (A, 0, Double_Type);
test_cumsum (A, 1, Double_Type);
test_cumsum (A, 2, Double_Type);

A = Char_Type[10]; A[*] = 1;
test_cumsum (A, -1, Float_Type);
test_cumsum (A, 0, Float_Type);
A = [1:3*4*5]; A = typecast (A, Char_Type);
reshape (A, [3,4,5]);
test_cumsum (A, -1, Float_Type);
test_cumsum (A, 0, Float_Type);
test_cumsum (A, 1, Float_Type);
test_cumsum (A, 2, Float_Type);

A = UChar_Type[10]; A[*] = 1;
test_cumsum (A, -1, Float_Type);
test_cumsum (A, 0, Float_Type);
A = [1:3*4*5]; A = typecast (A, UChar_Type);
reshape (A, [3,4,5]);
test_cumsum (A, -1, Float_Type);
test_cumsum (A, 0, Float_Type);
test_cumsum (A, 1, Float_Type);
test_cumsum (A, 2, Float_Type);

A = Short_Type[10]; A[*] = 1;
test_cumsum (A, -1, Float_Type);
test_cumsum (A, 0, Float_Type);
A = [1:3*4*5]; A = typecast (A, Short_Type);
reshape (A, [3,4,5]);
test_cumsum (A, -1, Float_Type);
test_cumsum (A, 0, Float_Type);
test_cumsum (A, 1, Float_Type);
test_cumsum (A, 2, Float_Type);

A = UShort_Type[10]; A[*] = 1;
test_cumsum (A, -1, Float_Type);
test_cumsum (A, 0, Float_Type);
A = [1:3*4*5]; A = typecast (A, UShort_Type);
reshape (A, [3,4,5]);
test_cumsum (A, -1, Float_Type);
test_cumsum (A, 0, Float_Type);
test_cumsum (A, 1, Float_Type);
test_cumsum (A, 2, Float_Type);

A = Float_Type[10]; A[*] = 1;
test_cumsum (A, -1, Float_Type);
test_cumsum (A, 0, Float_Type);
A = [1:3*4*5]*1.0f;
reshape (A, [3,4,5]);
test_cumsum (A, -1, Float_Type);
test_cumsum (A, 0, Float_Type);
test_cumsum (A, 1, Float_Type);
test_cumsum (A, 2, Float_Type);

#ifexists Complex_Type
A = Complex_Type[10]; A[*] = 1;
test_cumsum (A, -1, Complex_Type);
test_cumsum (A, 0, Complex_Type);
A = [1:3*4*5] + 2i*[1:3*4*5];
reshape (A, [3,4,5]);
test_cumsum (A, -1, Complex_Type);
test_cumsum (A, 0, Complex_Type);
test_cumsum (A, 1, Complex_Type);
test_cumsum (A, 2, Complex_Type);
#endif

#endif

print ("Ok\n");

exit (0);

