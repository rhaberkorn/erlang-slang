_debug_info = 1; () = evalfile ("inc.sl");

print ("Testing Associative Arrays ...");

static define key_to_value (k)
{
   return "<<<" + k + ">>>";
}

static define value_to_key (v)
{
   strcompress (v, "<>");
}

static define add_to_x (x, k)
{
   x[k] = key_to_value(k);
}

static variable Default_Value = "****Default-Value****";

define setup (type)
{
   variable x = Assoc_Type [type, Default_Value];
   
   add_to_x (x, "foo");
   add_to_x (x, "bar");
   add_to_x (x, "silly");
   add_to_x (x, "cow");
   add_to_x (x, "dog");
   add_to_x (x, "chicken");

   return x;
}

static variable X;

% Test create/destuction of arrays
loop (20) X = setup (Any_Type);

loop (20) X = setup (String_Type);
   
static variable k, v;

foreach (X)
{
   (k, v) = ();
   if ((k != value_to_key(v)) or (v != key_to_value (k))
       or (X[k] != v))
     failed ("foreach");
}

foreach (X) using ("keys")
{
   k = ();
   if (X[k] != key_to_value (k))
     failed ("foreach using keys");
}

foreach (X) using ("keys", "values")
{
   (k, v) = ();
   if ((k != value_to_key(v)) or (v != key_to_value (k))
       or (X[k] != v))
     failed ("foreach using keys, values");
}

k = assoc_get_keys (X);
v = assoc_get_values (X);

static variable i;
_for (0, length(k)-1, 1)
{
   i = ();
   if (v[i] != X[k[i]])
     failed ("assoc_get_keys/values");
   assoc_delete_key (X, k[i]);
}

if (length (X) != 0)
  error ("assoc_delete_key failed");

if (X["*******************"] != Default_Value)
  failed ("default value");

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


static define store_and_test (a, indx, value)
{
   a[indx] = value;
   if (typeof (value) != typeof(a[indx]))
     failed ("typeof (value)");
   if (neqs (a[indx], value))
     failed ("a[indx] != value");
}
   
X = Assoc_Type[];

store_and_test (X, "string", "string");
store_and_test (X, "array", ["a", "b", "c"]);
store_and_test (X, "int", 3);
#ifexists Complex_Type
store_and_test (X, "z", 3+2i);
#endif

static variable V = assoc_get_values (X);
static variable K = assoc_get_keys (X);

static variable i;

_for (0, length(X)-1, 1)
{
   i=();
   if (neqs(X[K[i]], @V[i]))
     failed ("assoc_get_values");
}

print ("Ok\n");

exit (0);

