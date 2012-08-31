/*

Copyright (c) 2005-2012, University of Oxford.
All rights reserved.

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
 * Neither the name of the University of Oxford nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef ABSTRACTHDF5CONVERTER_HPP_
#define ABSTRACTHDF5CONVERTER_HPP_

#include <string>
#include "AbstractTetrahedralMesh.hpp"
#include "OutputFileHandler.hpp"
#include "Hdf5DataReader.hpp"

/**
 * The derived children of this class convert output from Hdf5 format to
 * a range of other formats for postprocessing.
 */
template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
class AbstractHdf5Converter
{
protected:

    /** Pointer to reader of the file to be converted. */
    Hdf5DataReader* mpReader;

    /** Number of variables to output. Read from the reader. */
    unsigned mNumVariables;

    /** Base name for the files: [basename].vtu, [basename].dat etc.*/
    std::string mFileBaseName;

    /** Pointer to a mesh. */
    AbstractTetrahedralMesh<ELEMENT_DIM,SPACE_DIM>* mpMesh;

    /** Initialised as the directory in which to store the results. */
    OutputFileHandler* mpOutputFileHandler;

    /**
     * Get the subdirectory in which the converted output is stored,
     * relative to the input directory.
     */
    std::string mRelativeSubdirectory;

public:

    /**
     * Constructor, which does the conversion and writes the .info file.
     *
     * @note This method is collective, and must be called by all processes.
     *
     * @param inputDirectory The input directory, relative to CHASTE_TEST_OUTPUT, where the .h5 file has been written
     * @param fileBaseName The base name of the data file.
     * @param pMesh Pointer to the mesh.
     * @param subdirectoryName name for the output directory to be created (relative to inputDirectory)
     * @param datasetName  The name of the dataset to convert, defaults to "Data" which gives the PDE variables.
     */
    AbstractHdf5Converter(std::string inputDirectory,
                          std::string fileBaseName,
                          AbstractTetrahedralMesh<ELEMENT_DIM,SPACE_DIM>* pMesh,
                          std::string subdirectoryName,
                          std::string datasetName = "Data");

    /**
     * Destructor.
     */
    ~AbstractHdf5Converter();

    /**
     * Get the relative path of the subdirectory in which the converted output is stored.
     */
    std::string GetSubdirectory();
};

#endif /*ABSTRACTHDF5CONVERTER_HPP_*/
