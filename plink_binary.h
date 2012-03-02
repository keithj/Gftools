#ifndef PLINK_BIN
#define PLINK_BIN

#include <string>
#include <vector>
#include <map>
#include <set>
#include <fcntl.h>
#include <sys/mman.h>
#include "snp.h"
#include "individual.h"
#include "exceptions.h"

class plink_binary {
private:
    bool open_for_write;
    std::fstream *bed_file;

    // for bed file
    bool is_mem_mapped;
    char *fmap;
    size_t flen;
    int fd;
    unsigned int snp_ptr;      // index to next snp to be read
    unsigned int bytes_per_snp;

    void read_bed_header();

    void write_bed_header();

    void bed_write(gftools::snp s, std::vector<int> genotypes);

    void uncompress_calls(char *buffer, size_t len, std::vector<int> &genotypes);

    void compress_calls(char *buffer, std::vector<int> genotypes);

    void open_bed_write(std::string filename);

    void open_bed_read(std::string filename, bool quell_mem_mapping);

    void get_snp(size_t pos, std::vector<int> &genotypes);

    void extract_bed(size_t pos, size_t len, char *buffer);

    void init(std::string dataset, bool mode);

    /** Collates genotype call strings to determine the alleles involved.
     *
     * Splits each call string to determine the alleles involved. Checks the
     * number of alleles involved.
     * @param g_str A vector of genotype call strings.
     * @return A pair of allele strings, A and B.
     */
    std::vector<std::string> collate_alleles(const std::vector<std::string> &g_str);

    bool is_empty(std::ifstream &ifstream);

    std::string error_message();

public:
    /// The dataset name.
    std::string dataset;
    /// The character used to represent an allele where the genotype is uknown
    char missing_genotype;
    /// Prevent memory-mapping of the BED file, if true.
    bool quell_mem_mapping;
    /// The vector of SNPs.
    std::vector<gftools::snp> snps;
    /// The vector of individuals.
    std::vector<gftools::individual> individuals;
    /// An index of SNPs by name, mapping the name to an index in the BED data.
    std::map<std::string, int> snp_index;

    /** Constructor that creates and initializes named dataset.
     * Implicitly opens the dataset in read mode.
     *
     * @param dataset The dataset name.
     * @see open.
     */
    plink_binary(std::string dataset);

    /** Constructor that creates an unnamed, uninitialized instance.
     */
    plink_binary();

    /** The destructor.
     */
    ~plink_binary();

    /** Initializes a named Plink dataset.
     *
     * @param dataset The dataset name.
     * @param mode A read/write mode, true meaning write.
     */
    void open(std::string dataset, bool mode);

    /** Initializes a named Plink dataset in read mode.
     *
     * @param dataset The dataset name.
     */
    void open(std::string dataset);

    /** Closes an initialized Plink dataset.
     */
    void close(void);

    /** Populates a SNP vector with SNP data from a Plink BIM format file.
     *
     * The BIM file name is calculated from the current dataset name. This
     * file is opened, parsed and then closed.
     *
     * @param snps A reference to a vector of SNPs.
     */
    void read_bim(std::vector<gftools::snp> &snps);

    /** Writes SNP data to a Plink BIM format file.
     *
     * The BIM file name is calculated from the current dataset name. This
     * file is opened, written and then closed.
     *
     * @param snps A reference to a vector of SNPs.
     */
    void write_bim(std::vector<gftools::snp> snps);

    /** Makes a SNP from a Plink BIM format record.
     *
     * @param record A string.
     * @return A SNP.
     */
    gftools::snp from_bim(std::string record);

    /** Makes a Plink BIM record string from a SNP.
     *
     * @param snp A SNP.
     * @return A BIM record.
     */
    std::string to_bim(gftools::snp snp);

    /** Populates a vector with individual data from a Plink FAM format file.
     *
     * The FAM file name is calculated from the current dataset name. This
     * file is opened, parsed and then closed.
     *
     * @param ind A reference to an individual.
     */
    void read_fam(std::vector<gftools::individual> &ind);

    /** Writes individual data to a Plink FAM format file.
     *
     * The FAM file name is calculated from the current dataset name. This
     * file is opened, written and then closed.
     *
     * @param ind A reference to an individual.
     */
    void write_fam(std::vector<gftools::individual> ind);

    /** Makes an individual from a Plink FAM record.
     *
     * @param record A string.
     * @return An individual.
     */
    gftools::individual from_fam(std::string record);

    /** Makes a Plink FAM record string from an individual.
     *
     * @param ind An individual.
     * @return A FAM record.
     */
    std::string to_fam(gftools::individual ind);

    /** Decodes the next SNP and its genotype calls from BED data.
     *
     * @param s A SNP reference that will be pointed to the next SNP.
     * @param genotypes A vector of strings indicating the genotype calls
     * for the SNP.
     * @return true if there are further SNPs.
     */
    bool next_snp(gftools::snp &s, std::vector<std::string> &genotypes);

    /** Looks up a SNP by name and updates a vector of genotype strings to the
     * calls for that SNP.
     *
     * @param snp A SNP name.
     * @param genotypes A vector of genotype call strings.
     */
    void read_snp(std::string snp, std::vector<std::string> &genotypes);

    /** Looks up a SNP by index in the BED data and updates a vector of genotype
     * call integer codes for that SNP.
     *
     * @param snp A SNP index.
     * @param genotypes A vector of genotype call codes.
     */
    void read_snp(int snp, std::vector<int> &genotypes);

    /** Writes the data of a SNP and its corresponding genotypes into the BED data.
     *
     * Also pushes the SNP onto the vector of SNPs as a side-effect, so it looks
     * like you don't want to call this at the wrong time. E.g. don't call
     * read_snp prior to this or data may get written at the wrong file offset.
     * This function writes at the current position in the BED data.
     *
     * @param s A snp whose data will be written.
     * @param genotypes A vector of genotype call integer codes.
     */
    void write_snp(gftools::snp s, std::vector<int> genotypes);

    /** Writes the data of a SNP and its corresponding genotypes into the BED data.
     *
     * @see write_snp(gftools::snp s, std::vector<int> genotypes)
     *
     * @param s A snp whose data will be written.
     * @param genotypes A vector of genotype call strings.
     */
    void write_snp(gftools::snp s, std::vector<std::string> genotypes);

    /** Translates integer representations of genotype calls for one SNP to their
     * corresponding string representations.
     *
     * Essentially this is the inverse of genotypes_atoi, except that the SNP is
     * not modified.
     *
     * @param s The SNP whose genotype strings are to be generated.
     * @param g_num A vector of genotype call integer codes, @see genotypes_atoi.
     * @param g_str A vector of genotype call strings, updated according to
     * the integer codes.
     */
    void genotypes_itoa(gftools::snp s, std::vector<int> g_num, std::vector<std::string> &g_str);

    /** Translates string representations of genotype calls for one SNP to their
     * corresponding integer representations and updates the alleles of the SNP.
     *
     * The character '0' is reserved to mean no-call in the string
     * representation, otherwise the character used to represent each allele is
     * unimportant. However, if more than two alleles are called, an error will
     * be raised.
     *
     * The integer codes created for each genotype are:
     *
     *  - AA: 1
     *  - AB: 2
     *  - BA: 2
     *  - BB: 3
     *
     * The alleles of the SNP are not updated if there is a no-call.
     *
     * @param s The SNP whose alleles may be modified.
     * @param g_str A vector of integers, updated according to the genptype calls
     * strings.
     * @param g_num A vector of genotype call strings for the SNP, each allele
     * being a single character.
     */
    void genotypes_atoi(gftools::snp &s, std::vector<std::string> g_str, std::vector<int> &g_num);

};

#endif // PLINK_BIN