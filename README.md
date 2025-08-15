# `domrec` (DOMinant / RECessive)

`domrec` is a very simple reencoder of plink BED/BIM/FAM files to dominant, recessive, or dominance deviation, so those modes can be tested with complex GWAS models that otherwise don't support them (i.e. generalized mixed-effects models).
This c++ project converts genotypes on the fly, using practically no memory even for very large files.

# Compliation from source

This project depends on the standard c++ library only, and can be compiled as usual:
```bash
make
```
If successful, you can test that outputs are as expected by comparing to an internal R version (requires `genio` CRAN package) that is more straighforward code but consumes way more memory on larger files (it is tested on a toy dataset where this is not a concern).  The test passed if there are no error messages:
```bash
make test
```

Under `build/` there is a version already compiled under my own Linux computer, which may work with other Linux systems.

# Usage

`domrec` requires these three arguments:
```bash
domrec <input_base> <output_base> <mode>
```

- `<input_base>` is the shared base name of the input plink `.bed`, `.bim`, and `.fam` files, all of which are required.
- `<output_base>` is the base name of the output file.  `.bed` extension is automatically added, but `.bim` and `.fam` are not created (they are not modified by this reencoding, so they can simply be copied or linked from the input).
- `<mode>` must equal one of these three choices, which describes the transformation:
  - `dom`: dominant encoding (changes heterozygotes (x=1) to effect allele homozygotes (x=2))
  - `rec`: recessive encoding (changes heterozygotes (x=1) to non-effect allele homozygotes (x=0))
  - `dev`: dominance deviation encoding (changes heterozygotes (x=1) to effect allele homozygotes (x=2) and effect allele homozygotes (x=2) to non-effect allele homozygotes (x=0))
