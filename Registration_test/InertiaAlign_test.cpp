/******************************************************************************
*                 SOFA, Simulation Open-Framework Architecture                *
*                    (c) 2006 INRIA, USTL, UJF, CNRS, MGH                     *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this program. If not, see <http://www.gnu.org/licenses/>.        *
*******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#include <SofaTest/Sofa_test.h>
#include <Registration/InertiaAlign.h>
#include <SceneCreator/SceneCreator.h>
#include <SofaLoader/MeshOBJLoader.h>
#include <SofaLoader/MeshOBJLoader.h>
#include <SofaEngine/GenerateRigidMass.h>

#include <SofaSimulationGraph/DAGSimulation.h>
#include <sofa/simulation/Node.h>
#include <sofa/helper/RandomGenerator.h>

#include <assert.h>

using sofa::core::objectmodel::New;

//#include <projects/GenerateRigid/GenerateRigid.h>
namespace sofa{
using namespace modeling;


    struct InertiaAlign_test : public Sofa_test<>
    {
        typedef component::InertiaAlign InertiaAlign;
        typedef component::loader::MeshOBJLoader MeshOBJLoader;
        typedef component::engine::GenerateRigidMass<defaulttype::Rigid3dTypes,defaulttype::Rigid3Mass> GenerateRigidMass;


        bool translation_test()
        {
            bool res = true;
            SReal epsilon = 0.00001;
            //root = sofa::core::objectmodel::SPtr_dynamic_cast<sofa::simulation::Node>( sofa::simulation::getSimulation()->load(std::string(FLEXIBLE_TEST_SCENES_DIR) + "/" + "InertiaAlign.scn");
            helper::io::Mesh meshSource;
            MeshOBJLoader::SPtr meshLoaderSource = New<MeshOBJLoader>();
            meshLoaderSource->m_filename.setValue("/home/pierre/Workspace/boxes.obj");
            //TODO : changer ces chemins et potentiellement le modele
            meshLoaderSource->load();
            meshSource.Create("/home/pierre/Workspace/boxes.obj");
            helper::io::Mesh meshTarget;
            MeshOBJLoader::SPtr meshLoaderTarget = New<MeshOBJLoader>();
            meshLoaderTarget->m_filename.setValue("/home/pierre/Workspace/boxes.obj");
            meshLoaderTarget->load();
            meshTarget.Create("/home/pierre/Workspace/boxes.obj");

            /// Seed for random value
            long seed = 7;

            /// Random generator
            helper::RandomGenerator randomGenerator;
            randomGenerator.initSeed(seed);

            SReal translation_x,translation_y,translation_z;
            translation_x = randomGenerator.random<SReal>(-10.0,10.0);
            translation_y = randomGenerator.random<SReal>(-10.0,10.0);
            translation_z = randomGenerator.random<SReal>(-10.0,10.0);

            SReal rotation_x,rotation_y,rotation_z;
            rotation_x = randomGenerator.random<SReal>(-90.0,90.0);
            rotation_y = randomGenerator.random<SReal>(-90.0,90.0);
            rotation_z = randomGenerator.random<SReal>(-90.0,90.0);

            /// Compute Inertia Matrix
            meshLoaderSource->applyTranslation(translation_x, translation_y, translation_z);
            meshLoaderSource->applyRotation(rotation_x, rotation_y, rotation_z);
            defaulttype::Rigid3Mass massSource;
            type::Vec3d centerSource ;

            //GenerateRigid (massSource, centerSource, &meshSource);

            type::Vec3d centerTarget ;
            sofa::type::Vec3 translation;
            defaulttype::Rigid3Mass massTarget;

            GenerateRigidMass::SPtr rigidSource = New<GenerateRigidMass>();
            rigidSource->m_positions.setValue(meshLoaderSource->positions);
            rigidSource->m_triangles.setValue(meshLoaderSource->triangles);
            rigidSource->init();
            rigidSource->update();


            GenerateRigidMass::SPtr rigidTarget = New<GenerateRigidMass>();
            rigidTarget->m_positions.setValue(meshSource.getVertices());
            rigidTarget->init();
            rigidTarget->update();

            InertiaAlign::SPtr Ia = New<InertiaAlign>();
            //translatedPositions, centerTarget , centerSource, &translation ,massSource.inertiaMatrix,massTarget.inertiaMatrix );
;
            Ia->targetC.setValue(centerTarget);
            Ia->sourceC.setValue(centerSource);
            Ia->translation.setValue(translation);
            Ia->m_positions.setValue(meshLoaderSource.position);
            Ia->m_positiont.setValue(meshLoaderTarget.position);

            //Ia->targetInertiaMatrix.setValue(rigidSource.inertiaMatrix.getValue());
            //Ia->sourceInertiaMatrix.setValue(rigidTarget.inertiaMatrix.getValue());
            Ia->init();
            for (unsigned int i=0; i<translatedPositions.size();i++)
            {
                SReal diff = abs(translatedPositions[i][0] - meshSource.getVertices()[i][0]);
                if(diff>epsilon)
                    res = false;
                diff = abs(translatedPositions[i][1] - meshSource.getVertices()[i][1]);
                if(diff>epsilon)
                    res = false;
                diff = abs(translatedPositions[i][2] - meshSource.getVertices()[i][2]);
                if(diff>epsilon)
                    res = false;
            }
            return res;
        }

    };
    TEST_F(InertiaAlign_test, InertiaAlign){    ASSERT_TRUE(translation_test());  }

}


