# to make errors fatal
set -e

# run from tests dir
cd tests

# file base to use
name=dummy-33-101-0.1

# run all modes on this toy data
../build/domrec $name $name-dom dom
../build/domrec $name $name-rec rec
../build/domrec $name $name-dev dev

# make versions with R, where the coding is more straightforward
Rscript domrec.R $name $name-dom-R dom
Rscript domrec.R $name $name-rec-R rec
Rscript domrec.R $name $name-dev-R dev

# the way the files are made, we expect perfect equality!!!
diff -q $name-dom{,-R}.bed
diff -q $name-rec{,-R}.bed
diff -q $name-dev{,-R}.bed

# clean up if successful
rm $name-{dom,rec,dev}{,-R}.bed
