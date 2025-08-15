# a pure R version of domrec, easy to understand, but inefficient for very large files
library(genio)

# get arguments
args <- commandArgs( trailingOnly = TRUE )
base_in <- args[1]
base_out <- args[2]
mode <- args[3]

# skip regular checks, this is for internal use only

# load original data
data <- read_plink( base_in, verbose = FALSE )
X <- data$X

# apply desired transformation
if ( mode == 'dom' ) {
    X[ X == 1 ] <- 2
} else if ( mode == 'rec' ) {
    X[ X == 1 ] <- 0
} else if ( mode == 'dev' ) {
    X[ X == 2 ] <- 0
    X[ X == 1 ] <- 2
}

# save edited genotype data to output
write_bed( base_out, X, verbose = FALSE )
