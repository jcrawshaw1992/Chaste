/*

Copyright (c) 2005-2016, University of Oxford.
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

#include "AbstractPdeModifier.hpp"
#include "VtkMeshWriter.hpp"
#include "ReplicatableVector.hpp"
#include "PetscTools.hpp"
#include "AveragedSourceEllipticPde.hpp"
#include "AveragedSourceParabolicPde.hpp"

template<unsigned DIM>
AbstractPdeModifier<DIM>::AbstractPdeModifier(AbstractLinearPde<DIM,DIM>* pPde,
                                              AbstractBoundaryCondition<DIM>* pBoundaryCondition,
                                              bool isNeumannBoundaryCondition,
                                              bool deleteMemberPointersInDestructor,
                                              Vec solution)
    : AbstractCellBasedSimulationModifier<DIM>(),
      mpPde(pPde),
      mpBoundaryCondition(pBoundaryCondition),
      mIsNeumannBoundaryCondition(isNeumannBoundaryCondition),
      mDeleteMemberPointersInDestructor(deleteMemberPointersInDestructor),
      mSolution(NULL),
      mOutputDirectory(""),
      mOutputGradient(false),
      mOutputSolutionAtPdeNodes(false)
{
    if (solution)
    {
        mSolution = solution;
    }
}

template<unsigned DIM>
AbstractPdeModifier<DIM>::~AbstractPdeModifier()
{
}

template<unsigned DIM>
AbstractLinearPde<DIM,DIM>* AbstractPdeModifier<DIM>::GetPde() const
{
    return mpPde;
}

template<unsigned DIM>
AbstractBoundaryCondition<DIM>* AbstractPdeModifier<DIM>::GetBoundaryCondition() const
{
    return mpBoundaryCondition;
}

template<unsigned DIM>
bool AbstractPdeModifier<DIM>::IsNeumannBoundaryCondition()
{
    return mIsNeumannBoundaryCondition;
}

template<unsigned DIM>
void AbstractPdeModifier<DIM>::SetDependentVariableName(const std::string& rName)
{
    mDependentVariableName = rName;
}

template<unsigned DIM>
std::string& AbstractPdeModifier<DIM>::rGetDependentVariableName()
{
    return mDependentVariableName;
}

template<unsigned DIM>
bool AbstractPdeModifier<DIM>::HasAveragedSourcePde()
{
    return ((dynamic_cast<AveragedSourceEllipticPde<DIM>*>(mpPde) != NULL) ||
            (dynamic_cast<AveragedSourceParabolicPde<DIM>*>(mpPde) != NULL));
}

template<unsigned DIM>
void AbstractPdeModifier<DIM>::SetUpSourceTermsForAveragedSourcePde(TetrahedralMesh<DIM,DIM>* pMesh, std::map<CellPtr, unsigned>* pCellPdeElementMap)
{
    assert(HasAveragedSourcePde());
    if (dynamic_cast<AveragedSourceEllipticPde<DIM>*>(mpPde) != NULL)
    {
        static_cast<AveragedSourceEllipticPde<DIM>*>(mpPde)->SetupSourceTerms(*pMesh, pCellPdeElementMap);
    }
    else if (dynamic_cast<AveragedSourceParabolicPde<DIM>*>(mpPde) != NULL)
    {
        static_cast<AveragedSourceParabolicPde<DIM>*>(mpPde)->SetupSourceTerms(*pMesh, pCellPdeElementMap);
    }
}

template<unsigned DIM>
Vec AbstractPdeModifier<DIM>::GetSolution()
{
    return mSolution;
}

template<unsigned DIM>
Vec AbstractPdeModifier<DIM>::GetSolution() const
{
    return mSolution;
}

template<unsigned DIM>
void AbstractPdeModifier<DIM>::SetSolution(Vec solution)
{
    mSolution = solution;
}

template<unsigned DIM>
TetrahedralMesh<DIM,DIM>* AbstractPdeModifier<DIM>::GetFeMesh() const
{
    return mpFeMesh;
}

template<unsigned DIM>
void AbstractPdeModifier<DIM>::SetupSolve(AbstractCellPopulation<DIM,DIM>& rCellPopulation, std::string outputDirectory)
{
    // Cache the output directory
    this->mOutputDirectory = outputDirectory;

    if (PetscTools::AmMaster())
    {
        OutputFileHandler output_file_handler(outputDirectory+"/", false);
        mpVizPdeSolutionResultsFile = output_file_handler.OpenOutputFile("results.vizpdesolution");
    }
}

template<unsigned DIM>
void AbstractPdeModifier<DIM>::UpdateAtEndOfOutputTimeStep(AbstractCellPopulation<DIM,DIM>& rCellPopulation)
{
    if (mOutputSolutionAtPdeNodes)
    {
        if (PetscTools::AmMaster())
        {
            (*mpVizPdeSolutionResultsFile) << SimulationTime::Instance()->GetTime() << "\t";

            if (mpFeMesh != NULL)
            {
                assert(mDependentVariableName != "");

                for (unsigned i=0; i<mpFeMesh->GetNumNodes(); i++)
                {
                    (*mpVizPdeSolutionResultsFile) << i << " ";
                    c_vector<double,DIM> location = mpFeMesh->GetNode(i)->rGetLocation();
                    for (unsigned k=0; k<DIM; k++)
                    {
                        (*mpVizPdeSolutionResultsFile) << location[k] << " ";
                    }

                    assert(mSolution != NULL);
                    ReplicatableVector solution_repl(mSolution);
                    (*mpVizPdeSolutionResultsFile) << solution_repl[i] << " ";
                }
            }
            else // Not coarse mesh
            {
                for (typename AbstractCellPopulation<DIM>::Iterator cell_iter = rCellPopulation.Begin();
                     cell_iter != rCellPopulation.End();
                     ++cell_iter)
                {
                    unsigned node_index = rCellPopulation.GetLocationIndexUsingCell(*cell_iter);
                    (*mpVizPdeSolutionResultsFile) << node_index << " ";
                    const c_vector<double,DIM>& position = rCellPopulation.GetLocationOfCellCentre(*cell_iter);
                    for (unsigned i=0; i<DIM; i++)
                    {
                        (*mpVizPdeSolutionResultsFile) << position[i] << " ";
                    }
                    double solution = cell_iter->GetCellData()->GetItem(mDependentVariableName);
                    (*mpVizPdeSolutionResultsFile) << solution << " ";
                }
            }
            (*mpVizPdeSolutionResultsFile) << "\n";
        }
    }
#ifdef CHASTE_VTK
    if (DIM > 1)
    {
        std::ostringstream time_string;
        time_string << SimulationTime::Instance()->GetTimeStepsElapsed();
        std::string results_file = "pde_results_" + mDependentVariableName + "_" + time_string.str();
        VtkMeshWriter<DIM,DIM>* p_vtk_mesh_writer = new VtkMeshWriter<DIM,DIM>(mOutputDirectory, results_file, false);

        ReplicatableVector solution_repl(mSolution);
        std::vector<double> pde_solution;
        for (unsigned i=0; i<mpFeMesh->GetNumNodes(); i++)
        {
           pde_solution.push_back(solution_repl[i]);
        }

        p_vtk_mesh_writer->AddPointData(mDependentVariableName, pde_solution);

        p_vtk_mesh_writer->WriteFilesUsingMesh(*mpFeMesh);
        delete p_vtk_mesh_writer;
    }
#endif //CHASTE_VTK
}

template<unsigned DIM>
void AbstractPdeModifier<DIM>::UpdateAtEndOfSolve(AbstractCellPopulation<DIM,DIM>& rCellPopulation)
{
    if (PetscTools::AmMaster())
    {
        mpVizPdeSolutionResultsFile->close();
    }
}

template<unsigned DIM>
bool AbstractPdeModifier<DIM>::GetOutputGradient()
{
    return mOutputGradient;
}

template<unsigned DIM>
void AbstractPdeModifier<DIM>::SetOutputGradient(bool outputGradient)
{
    mOutputGradient = outputGradient;
}

template<unsigned DIM>
void AbstractPdeModifier<DIM>::SetOutputSolutionAtPdeNodes(bool outputSolutionAtPdeNodes)
{
    mOutputSolutionAtPdeNodes = outputSolutionAtPdeNodes;
}

template<unsigned DIM>
void AbstractPdeModifier<DIM>::OutputSimulationModifierParameters(out_stream& rParamsFile)
{
    // No parameters to output, so just call method on direct parent class
    AbstractCellBasedSimulationModifier<DIM>::OutputSimulationModifierParameters(rParamsFile);
}

// Explicit instantiation
template class AbstractPdeModifier<1>;
template class AbstractPdeModifier<2>;
template class AbstractPdeModifier<3>;
