_debug_info = 1; () = evalfile ("inc.sl");

print ("Testing string functions...");

variable s;

s = strcompress (" \t  \tA\n\ntest\t", " \t\n");
if (s != "A test") failed ("strcompress");

s = " \t hello world\n\t";
if ("hello world" != strtrim (s)) failed ("strtrim");
if ("hello world\n\t" != strtrim_beg (s)) failed ("strtrim_beg");
if (" \t hello world" != strtrim_end (s)) failed ("strtrim_beg");

if ("hello wor" != strtrim (s, " \t\nld")) failed ("strtrim with whitespace");

if ("" != strcat ("", ""))
  failed ("strcat 0");
if ("1" != strcat ("", "1"))
  failed ("strcat 1");

if ("abcdefg" != strcat ("a", "b", "c", "d", "e", "f", "g")) failed ("strcat");
if ("abcdefg" != strcat ("abcdefg")) failed ("strcat 2");

if ((strtok (s)[0] != "hello") 
    or (strtok(s)[1] != "world")
    or (strtok (s, "^a-z")[0] != "hello")
    or (strtok (s, "^a-z")[1] != "world")
    or (2 != length (strtok (s)))
    or (2 != length (strtok (s, "^a-z")))) failed ("strtok");

define test_create_delimited_string ()
{
   variable n = ();
   variable args = __pop_args (_NARGS - 3);
   variable delim = ();
   variable eresult = ();
   variable result;
   
   result = create_delimited_string (delim, __push_args (args), n);
   if (eresult != result)
     failed ("create_delimited_string: expected: %s, got: %s",
	     eresult, result);

   if (n)
     result = strjoin ([__push_args (args)], delim);
   else 
     result = strjoin (String_Type[0], delim);

   if (eresult != result)
     failed ("strjoin: expected: %s, got: %s",
	     eresult, result);
}

	
test_create_delimited_string ("aXXbXXcXXdXXe",
			      "XX",
			      "a", "b", "c", "d", "e",
			      5);


test_create_delimited_string ("", "", "", 1);
test_create_delimited_string ("a", ",", "a", 1);
test_create_delimited_string (",", ",", "", "", 2);
test_create_delimited_string (",,", ",", "", "", "", 3);
test_create_delimited_string ("", "XXX", 0);

static define test_strtrans (s, from, to, ans)
{
   variable s1 = strtrans (s, from, to);
   if (ans != s1)
     failed ("strtrans(%s, %s, %s) --> %s", s, from, to, s1);
}

test_strtrans ("hello world", "^a-zA-Z", "X", "helloXworld");
test_strtrans ("hello", "", "xxxx", "hello");
test_strtrans ("hello", "l", "", "heo");
test_strtrans ("hello", "helo", "abcd", "abccd");
test_strtrans ("hello", "hl", "X", "XeXXo");
test_strtrans ("", "hl", "X", "");
test_strtrans ("hello", "a-z", "A-Z", "HELLO");
test_strtrans ("hello", "a-mn-z", "A-MN-Z", "HELLO");
test_strtrans ("abcdefg", "a-z", "Z-A", "ZYXWVUT");
test_strtrans ("hejklo", "k-l", "L-L---", "hejL-o");
test_strtrans ("hello", "he", "-+", "-+llo");
test_strtrans ("hello", "", "", "hello");
test_strtrans ("hello", "helo", "", "");
test_strtrans ("hello", "o", "", "hell");
test_strtrans ("hello", "hlo", "", "e");
test_strtrans ("", "hlo", "", "");
test_strtrans ("HeLLo", "A-Ze", "", "o");
test_strtrans ("HeLLo", "^A-Z", "", "HLL");

define test_str_replace (a, b, c, result, n)
{
   variable new;
   variable m;

   (new, m) = strreplace (a, b, c, n);

   if (new != result)
     failed ("strreplace (%s, %s, %s, %d) ==> %s", a, b, c, n, new);
   
   if (n == 1)
     {
	n = str_replace (a, b, c);
	!if (n) a;
	new = ();
	if (new != result)
	  failed ("str_replace (%s, %s, %s) ==> %s", a, b, c, new);
     }
}

test_str_replace ("a", "b", "x", "a", 1);
test_str_replace ("a", "b", "x", "a", -1);
test_str_replace ("a", "b", "x", "a", -10);
test_str_replace ("a", "b", "x", "a", 10);
test_str_replace ("a", "b", "x", "a", 0);
test_str_replace ("blafoofbarfoobar", "", "xyyy", "blafoofbarfoobar", 0);
test_str_replace ("blafoofbarfoobar", "", "xyyy", "blafoofbarfoobar", 1);
test_str_replace ("blafoofbarfoobar", "", "xyyy", "blafoofbarfoobar", -1);
test_str_replace ("blafoofbarfoobar", "", "xyyy", "blafoofbarfoobar", -10);

test_str_replace ("blafoofbarfoobar", "foo", "XY", "blafoofbarfoobar", 0);
test_str_replace ("blafoofbarfoobar", "foo", "XY", "blaXYfbarfoobar", 1);
test_str_replace ("blafoofbarfoobar", "foo", "XY", "blaXYfbarXYbar", 2);
test_str_replace ("blafoofbarfoobar", "foo", "XY", "blaXYfbarXYbar", 10);
test_str_replace ("blafoofbarfoobar", "foo", "XY", "blafoofbarXYbar", -1);
test_str_replace ("blafoofbarfoobar", "foo", "XY", "blaXYfbarXYbar", -2);
test_str_replace ("blafoofbarfoobar", "r", "", "blafoofbarfoobar", 0);
test_str_replace ("blafoofbarfoobar", "r", "", "blafoofbafoobar", 1);
test_str_replace ("blafoofbarfoobar", "r", "", "blafoofbafooba", 2);
test_str_replace ("blafoofbarfoobar", "r", "", "blafoofbarfooba", -1);
test_str_replace ("blafoofbarfoobar", "r", "", "blafoofbafooba", -2);
test_str_replace ("bla", "bla", "", "", -2);
test_str_replace ("bla", "bla", "foo", "foo", -2);
test_str_replace ("bla", "bla", "foo", "foo", 1);

define test_strcat ()
{
   % This test generates a combined byte-code.  It is used for leak checking
   variable a = "hello";
   variable b = "world";
   loop (20)
     {
	variable c = a + b;
	a = c;
     }
}

		
print ("Ok\n");
exit (0);
