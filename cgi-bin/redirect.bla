#!/usr/local/bin/perl -wT
# redirect.pl -- displays form data
BEGIN {my $homedir = ( getpwuid($>) )[7];my @user_include;foreach my $path (@INC) { if ( -d $homedir . '/perl' . $path ) 
  {push @user_include, $homedir . '/perl' . $path;}} unshift @INC, @user_include;}
use warnings;
use strict;
use CGI qw(:standard);
use CGI::Carp qw(warningsToBrowser fatalsToBrowser);
my $form1 = new CGI;
my $FirstName = $form1->param('first_name');
my $LastName = $form1->param('last_name');
my $RedirectMethod = $form1->param('redirect-method');
print (qq~Content-Type: text/html
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <meta content="text/html; charset=utf-8" http-equiv="Content-Type" />
  <title>HTML redirect test</title>
</head>
<body>
  <h1>Data received</h1>
  <p>First Name: $FirstName</p>
  <p>Last Name: $LastName</p>
  <p>Redirect Method: $RedirectMethod</p>
</body>
</html>
~);