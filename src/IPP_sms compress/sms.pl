#!/usr/bin/perl

#SMS:xmolna02

# ------------------------------------------------------------------------------
# File: sms.pl
# Date: 2011/04/08
# Author: David Molnar, xmolna02@stud.fit.vutbr.cz
# Project: SMS Compress
# ------------------------------------------------------------------------------


use strict;
use locale;
use utf8;
use encoding 'utf-8';
use XML::Simple;


# process params
# ------------------------------------------------------------------------------

my %sw = process_params();

# print help to stdout
if (defined $sw{help})
{
  print_help();
  exit;
}


# init (open) I/O files
# ------------------------------------------------------------------------------

# open input file
if (defined $sw{_ifile})
{
  open(IFILE, "<:utf8", $sw{_ifile}) || 
    exit_error(2, "nelze otevrit vstupni soubor pro cteni!");
}

# open output file for writing
if (defined $sw{_ofile})
{
  open(OFILE, ">:utf8", $sw{_ofile}) || 
    exit_error(3, "nelze otevrit vystupni soubor pro zapis!");
}

# open xml dictionary file

my $dic;

if (defined $sw{_dfile})
{
  $dic = eval { XMLin($sw{_dfile}) };
  
  if ($@)
  {
    close IFILE;
    close OFILE;
    exit_error(1, "nelze otevrit/zpracovat XML slovnik!");
  }
}


# read everything from input file (or stdin) and save to $input
# ------------------------------------------------------------------------------

my $input = '';

while (my $row = defined $sw{input} ? <IFILE> : <STDIN>)
{
  $input .= $row;
}

if (defined $sw{input})
{
  close IFILE;
}

my $output = $input;


# r option - remove diacritics
# ------------------------------------------------------------------------------

if (defined $sw{r})
{
  $output = remove_diacritic($output);
}


# v option - from xml file
# ------------------------------------------------------------------------------

if (defined $sw{v})
{
  $output = process_dic($output, $dic, $sw{e}, $sw{s});
}


# c option - camel notation
# ------------------------------------------------------------------------------

if (defined $sw{c})
{
  $output = process_camel($output, $sw{a}, $sw{b});
}


# n option - get count of neccessary sms
# ------------------------------------------------------------------------------

my $sms_count = 0;

if (defined $sw{n})
{
  $sms_count = get_sms_count($input, $output);
}


# write results to file (or stdout)
# if switch -n is used then write only count of sms
# ------------------------------------------------------------------------------

my $result = defined $sw{n} ? \$sms_count : \$output;

if (defined $sw{output})
{
  print OFILE $$result;
  close OFILE;
}
else
{
  print $$result;
}

# functions
# ------------------------------------------------------------------------------

# print help to stdout
sub print_help
{
  print 
"SMS Compress
     - Komprese SMS
     - Odstraneni ceske diakritiky
     - Aplikace pravidel ze slovniku zkratek.

Parametry
    --help                  Vytiskne napovedu
                            (nelze s nicim)
    --input=filename.ext    Vstupni soubor obsahujici SMS
    --output=filename.text  Vystupni soubor s SMS v ASCII
    -r                      Odstraneni diakritiky z SMS
                            (lze -c a -v, odstraneni diakritiky se provede jako prvni)
    -c                      Camel komprese, viz dalsi nastaveni
    -a                      Komprimovat pouze slova z malych pismen
                            (lze s -c)
    -b                      Nekomprimovat slova z velkych pismen
                            (lze s -c, nelze s -a)    
    -dict=filename          Urceni XML slovniku zkratek
                            (nutne s -v)
    -v                      Aplikace pravidel ze slovniku zkratek
                            (nutne s -d, lze s -c, lze s -e nebo -s)
    -e                      Aplikace pouze expanzivnich pravidel
                            (nutne s -v, nelze s -s)
    -s                      Aplikace pouze zkracujicich pravidel
                            (nutne s -v, nelze s -e)
    -n                      Spocita minimalni pocet SMS\n";
}

# print error message to stderr and exit
sub exit_error
{
  print STDERR "Chyba: ", $_[1], "\n";
  exit $_[0];
}

# processes params from command line
sub process_params
{
  my %res = ();

  foreach my $item (@ARGV)
  {
    SWITCH: 
    {
      $item =~ /^--help$/         && do { $res{help} += 1; last SWITCH; };
      $item =~ /^--input=(.*?)$/  && do { $res{input} += 1;
                                          $res{_ifile} = $1; last SWITCH; };
      $item =~ /^--output=(.*?)$/ && do { $res{output} += 1;
                                          $res{_ofile} = $1; last SWITCH; };
      $item =~ /^--dict=(.*?)$/   && do { $res{dict}  += 1; 
                                          $res{_dfile} = $1; last SWITCH; };
      $item =~ /^-r$/             && do { $res{r} += 1; last SWITCH; };
      $item =~ /^-c$/             && do { $res{c} += 1; last SWITCH; };
      $item =~ /^-a$/             && do { $res{a} += 1; last SWITCH; };
      $item =~ /^-b$/             && do { $res{b} += 1; last SWITCH; };
      $item =~ /^-v$/             && do { $res{v} += 1; last SWITCH; };
      $item =~ /^-e$/             && do { $res{e} += 1; last SWITCH; };
      $item =~ /^-s$/             && do { $res{s} += 1; last SWITCH; };
      $item =~ /^-n$/             && do { $res{n} += 1; last SWITCH; };
      exit_error(1, "Neznamy parameter!");
    }
  }

  foreach my $item (keys %res)
  {
    if ($item =~ /^[^_]/ && $res{$item} > 1)
    {
      exit_error(1, "parametre nelze zadat opakovane!");
    }
  }

  if (defined $res{help} && scalar keys %res > 1)
    { exit_error(1, "--help nelze kombinovat inymi parametrami!"); }

  if (defined $res{a} && !defined $res{c})
    { exit_error(1, "-a lze s -c!"); }

  if (defined $res{b} && (!defined $res{c} || defined $res{a}))
    { exit_error(1, "-b lze s -c a nelze s -a!"); }

  if (defined $res{_dfile} && !defined $res{v})
    { exit_error(1, "--dict lze s -v!"); }

  if (defined $res{e} && !defined $res{v})
    { exit_error(1, "-e lze s -v!"); }

  if (defined $res{s} && (!defined $res{v} || defined $res{e}))
    { exit_error(1, "-s lze s -v a nelze s -e!"); }
  
  return %res;
}

# removes czech diacritics and returns the result
sub remove_diacritic
{
  # "dictionary" for removing czech diacritics characters 
  my %char_hash = (á => "a", č => "c", ď => "d", é => "e", ě => "e", í => "i", 
                   ň => "n", ó => "o", ř => "r", š => "s", ť => "t", ú => "u", 
                   ů => "u", ý => "y", ž => "z",
                   Á => "A", Č => "C", Ď => "D", É => "E", Ě => "E", Í => "I", 
                   Ň => "N", Ó => "O", Ř => "R", Š => "S", Ť => "T", Ú => "U",
                   Ů => "U", Ý => "Y", Ž => "Z");

  return join('', map { $a = $_; 
                        (grep(/\Q$a\E/, keys %char_hash))[0] 
                                                        ? $char_hash{$_} : $_ } 
                    split('', $_[0]));
}

# convert input to camel notation
sub process_camel
{
  my $result = $_[0];
  my $asw = $_[1];
  my $bsw = $_[2]; 

  # switch -a used: only words containing only lowercase
  if (defined $asw)
  {
    $result =~ s/(
                   ^             # beginning of the line
                   |             # or
                   [^[:alpha:]]  # one character what is not letter
                 )
                 (
                   [[:lower:]]+  # one or more lowercase letters
                 )
                 /$1\u$2/xg;
  }
  # switch -b used: skip words containing only uppercase
  # ()|() does not working therefore two s//
  elsif (defined $bsw)
  {
    $result =~ s/(
                   ^             # beginning of the line
                   |             # or
                   [^[:alpha:]]  # one character what is not letter
                 )
                 (
                   [[:upper:]]   # one uppercase
                 )
                 (
                                 # letters containing min. one lowercase
                   [[:upper:]]*[[:lower:]][[:alpha:]]*
                 )
                 /$1\u$2\L$3\E/xg; 
   
    $result =~ s/(
                   ^             # beginning of the line
                   |             # or
                   [^[:alpha:]]  # one character what is not letter
                 )
                 (
                   [[:lower:]]   # one lowercase
                 )
                 (
                   [[:alpha:]]*  # zero or more letters
                 )
                 /$1\u$2\L$3\E/xg;
  }
  else
  {
    $result =~ s/(
                   ^             # beginning of the line
                   |             # or
                   [^[:alpha:]]  # one character what is not letter
                 )
                 (
                   [[:alpha:]]   # one letter
                 )
                 (
                   [[:alpha:]]*  # zero or more letters
                 )
                 /$1\u$2\L$3\E/xg;
  }

  # clear all white space
  $result =~ s/[[:space:]]+//xg;

  return $result;
}

# process expansion or shortening based on xml dictionary file
sub process_dic
{
  my $result = $_[0];
  my $dic = $_[1];
  my $esw = $_[2];
  my $ssw = $_[3];

  # expansion
  if (defined $esw || (!defined $esw && !defined $ssw))
  {
    foreach my $item (@{$dic->{rule}})
    {
      next if (!defined $item->{expansive});
    
      my $abb = $item->{abbrev};
      my $text = $item->{text};
      my $case = $item->{casesensitive};      

      $result =~ s/\Q$abb\E/$text/gi if (!defined $case);
      $result =~ s/\Q$abb\E/$text/g if (defined $case && $case == 1);
    }
  }

  # shortening
  if (defined $ssw || (!defined $esw && !defined $ssw))
  {
    foreach my $item (@{$dic->{rule}})
    {
      next if (defined $item->{expansive} && $item->{expansive} == 1);

      my $abb = $item->{abbrev};
      my $text = $item->{text};
      my $case = $item->{casesensitive};      

      $result =~ s/\Q$text\E/$abb/gi if (!defined $case);
      $result =~ s/\Q$text\E/$abb/g if (defined $case && $case == 1);
    }
  }

  return $result;
}

# get count of necessary sms('s)
# max length of one sms is 160 (70 diacritics)
# max length if more sms used 153 (67 diacritics)
sub get_sms_count
{
  my $result = 0;
  my $len = length $_[1];

  if ($_[0] eq remove_diacritic($_[0]))
  {
    $result = ($len > 160) ? int($len / 153 + 0.99) : $len;
  }
  else
  {
    $result = ($len > 70) ? int($len / 67 + 0.99) : $len;
  }  

  return $result;
}


# end of sms.pl
# ------------------------------------------------------------------------------
