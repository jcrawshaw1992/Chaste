/*

Copyright (C) University of Oxford, 2005-2012

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Chaste is free software: you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Chaste is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details. The offer of Chaste under the terms of the
License is subject to the License being interpreted in accordance with
English Law and subject to any action against the University of Oxford
being under the jurisdiction of the English Courts.

You should have received a copy of the GNU Lesser General Public License
along with Chaste. If not, see <http://www.gnu.org/licenses/>.

*/

#include "Alarcon2004OxygenBasedCellCycleOdeSystem.hpp"
#include "CellwiseOdeSystemInformation.hpp"

Alarcon2004OxygenBasedCellCycleOdeSystem::Alarcon2004OxygenBasedCellCycleOdeSystem(double oxygenConcentration,
                                                                                   bool isLabelled,
                                                                                   std::vector<double> stateVariables)
    : AbstractOdeSystem(6),
      mOxygenConcentration(oxygenConcentration),
      mIsLabelled(isLabelled)
{
    mpSystemInfo.reset(new CellwiseOdeSystemInformation<Alarcon2004OxygenBasedCellCycleOdeSystem>);

    /**
     * State variables
     *
     * 0. x = Cdh1-APC complexes
     * 1. y = cyclin-CDK
     * 2. z = p27
     * 3. m = mass
     * 4. u = RBNP
     * 5. oxygenConcentration
     */
    Init(); // set up parameters

    // Parameter values are taken from the Alarcon et al. (2004) paper
    if (mIsLabelled) // labelled "cancer" cells
    {
        ma1 = 0.04;
        mc1 = 0.007;
        mxThreshold = 0.004;
        myThreshold = 0.05;
    }
    else // normal cells
    {
        ma1 = 0.05;
        mc1 = 0.1;
        mxThreshold = 0.004;
        myThreshold = 0.2;
    }

    // Cell-specific initial conditions
    SetDefaultInitialCondition(3, 0.5*mMstar);
    SetDefaultInitialCondition(5, oxygenConcentration);

    if (stateVariables != std::vector<double>())
    {
        SetStateVariables(stateVariables);
    }
}

Alarcon2004OxygenBasedCellCycleOdeSystem::~Alarcon2004OxygenBasedCellCycleOdeSystem()
{
    // Do nothing
}

void Alarcon2004OxygenBasedCellCycleOdeSystem::Init()
{
    // Parameter values are taken from the Alarcon et al. (2004) paper
    ma2 = 1.0;
    ma3 = 0.25;
    ma4 = 0.04;
    mb3 = 10.0;
    mb4 = 5.5;
    mc2 = 0.01;
    md1 = 0.01;
    md2 = 0.1;
    mJ3 = 0.04;
    mJ4 = 0.04;
    mEta = 0.01;
    mMstar = 10.0;
    mB = 0.01;
}

void Alarcon2004OxygenBasedCellCycleOdeSystem::EvaluateYDerivatives(double time, const std::vector<double>& rY, std::vector<double>& rDY)
{
    double x = rY[0];
    double y = rY[1];
    double z = rY[2];
    double mass = rY[3];
    double u = rY[4];
    double oxygen_concentration = rY[5];

    double dx = 0.0;
    double dy = 0.0;
    double dz = 0.0;
    double dmass = 0.0;
    double du = 0.0;

    /*
     % The variables are
     % 1. x = Cdh1-APC complexes
     % 2. y = cyclin-CDK
     % 3. z = p27
     % 4. m = mass
     % 5. u = RBNP
    */

    dx = ((1 + mb3*u)*(1-x))/(mJ3 + 1 - x) - (mb4*mass*x*y)/(mJ4 + x);
    dy = ma4 -(ma1 + ma2*x + ma3*z)*y;

    // Parameter values are taken from the Alarcon et al. (2004) paper
    if (mIsLabelled) // labelled "cancer" cells
    {
        dz = mc1 - mc2*oxygen_concentration*z/(mB + oxygen_concentration);
    }
    else // normal cells
    {
        dz = mc1*(1 - mass/mMstar) - mc2*oxygen_concentration*z/(mB + oxygen_concentration);
    }

    dmass = mEta*mass*(1 - mass/mMstar);
    du = md1 - (md2 + md1*y)*u;

    // Rescale time to be in hours
    rDY[0] = 60.0*dx;
    rDY[1] = 60.0*dy;
    rDY[2] = 60.0*dz;
    rDY[3] = 60.0*dmass;
    rDY[4] = 60.0*du;
    rDY[5] = 0.0; // do not change the oxygen concentration
}

bool Alarcon2004OxygenBasedCellCycleOdeSystem::CalculateStoppingEvent(double time, const std::vector<double>& rY)
{
    return (rY[0] < mxThreshold && rY[1] > myThreshold);
}

template<>
void CellwiseOdeSystemInformation<Alarcon2004OxygenBasedCellCycleOdeSystem>::Initialise()
{
    this->mVariableNames.push_back("Cdh1_APC_complexes");
    this->mVariableUnits.push_back("non_dim");
    this->mInitialConditions.push_back(0.9);

    this->mVariableNames.push_back("cyclin_CDK");
    this->mVariableUnits.push_back("non_dim");
    this->mInitialConditions.push_back(0.01);

    this->mVariableNames.push_back("p27");
    this->mVariableUnits.push_back("non_dim");
    this->mInitialConditions.push_back(0.0);

    this->mVariableNames.push_back("mass");
    this->mVariableUnits.push_back("non_dim");
    this->mInitialConditions.push_back(NAN); // will be filled in later

    this->mVariableNames.push_back("RBNP");
    this->mVariableUnits.push_back("non_dim");
    this->mInitialConditions.push_back(1.0);

    this->mVariableNames.push_back("O2");
    this->mVariableUnits.push_back("non_dim");
    this->mInitialConditions.push_back(NAN); // will be filled in later

    this->mInitialised = true;
}

void Alarcon2004OxygenBasedCellCycleOdeSystem::SetIsLabelled(bool isLabelled)
{
    mIsLabelled = isLabelled;
}

bool Alarcon2004OxygenBasedCellCycleOdeSystem::IsLabelled() const
{
    return mIsLabelled;
}

double Alarcon2004OxygenBasedCellCycleOdeSystem::GetOxygenConcentration() const
{
    return mOxygenConcentration;
}

// Serialization for Boost >= 1.36
#include "SerializationExportWrapperForCpp.hpp"
CHASTE_CLASS_EXPORT(Alarcon2004OxygenBasedCellCycleOdeSystem)
