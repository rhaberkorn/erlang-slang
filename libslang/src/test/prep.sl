_debug_info = 1; () = evalfile ("inc.sl");

print ("Testing slprep ...");

public variable X = 0;

#ifdef FOO_MOO_TOO_KOO
failed ("ifdef");
#else
X = 1;
#endif
#if (X!=1)
failed ("X!=1");
#else
X=-1;
#endif

#if !eval(X==-1)
failed ("eval");
#else
  
print ("Ok\n");

exit (0);
#endif
