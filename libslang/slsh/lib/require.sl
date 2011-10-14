% These functions were taken from the jed editor

static variable Features = Assoc_Type [Int_Type,0];

%!%+
%\function{_featurep}
%\synopsis{Test whether or not a feature is present}
%\usage{Int_Type _featurep (String_Type feature)}
%\description
%  The \sfun{_featurep} function returns a non-zero value if the specified 
%  feature is present.  Otherwise, it returns 0 to indicate that the feature
%  has not been loaded.
%\seealso{require, provide}
%!%-
public define _featurep (f)
{
   Features[f];
}


%!%+
%\function{provide}
%\synopsis{Declare that a specified feature is available}
%\usage{provide (String_Type feature)}
%\description
% The \sfun{provide} function may be used to declare that a "feature" has 
% been loaded.  See the documentation for \sfun{require} for more information.
%\seealso{require, _featurep}
%!%-
public define provide (f)
{
   Features[f] = 1;
}

%!%+
%\function{require}
%\synopsis{Make sure a feature is present, and load it if not}
%\usage{require (String_Type feature [,String_Type file]}
%\description
%  The \sfun{require} function ensures that a specified "feature" is present.
%  If the feature is not present, the \sfun{require} function will attempt to
%  load the feature from a file.  If called with two arguments, the feature
%  will be loaded from the file specified by the second argument.  Otherwise,
%  the feature will be loaded from a file given by the name of the feature, 
%  with ".sl" appended.
%  
%  If after loading the file, if the feature is not present, 
%  a warning message will be issued.
%\notes
%  "feature" is an abstract quantity that is undefined here.
%  
%  A popular use of the \sfun{require} function is to ensure that a specified
%  file has already been loaded.  In this case, the feature is the 
%  filename itself.  The advantage of using this mechanism over using 
%  \ifun{evalfile} is that if the file has already been loaded, \sfun{require}
%  will not re-load it.  For this to work, the file must indicate that it 
%  provides the feature via the \sfun{provide} function.
%\seealso{provide, _featurep, evalfile}
%!%-
public define require ()
{
   variable f, file;

   if (_NARGS == 2)
     {
	(f, file) = ();
     }
   else
     {
	f = ();
	file = f;
     }

   if (_featurep (f))
     return;

   () = evalfile (file);
   !if (_featurep (f))
     vmessage ("***Warning: feature %s not found in %s", f, file);
}


