#ifndef LIBRARY_ACCESS_GUTTED_H
#define LIBRARY_ACCESS_GUTTED_H

#include <string>
#include <vector>

//This file is designed to access the visibility parameters from the
//optical libraries, which are needed to calculate the number of photoelectrons
//incident on each PMT.

class LibraryAccess{

  public:
    void LoadLibraryFromFile(std::string libraryfile, bool reflected, bool reflT0);
    const float* GetReflT0(size_t Voxel, int no_pmt);
    const float* GetReflCounts(size_t Voxel, int no_pmt, bool is_reflT0);
    const float* GetCounts(size_t Voxel, int no_pmt);
    const float* GetLibraryEntries(int VoxID, bool wantReflected, int no_pmt);
    
    LibraryAccess();

  private:
    std::vector<std::vector<float> > table_;
    std::vector<std::vector<float> > reflected_table_;
    std::vector<std::vector<float> > reflT_table_;
};

#endif
