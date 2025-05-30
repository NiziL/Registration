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
#ifndef SOFA_COMPONENT_INTERACTIONFORCEFIELD_REGISTRATIONCONTACTFORCEFIELD_INL
#define SOFA_COMPONENT_INTERACTIONFORCEFIELD_REGISTRATIONCONTACTFORCEFIELD_INL

#include "RegistrationContactForceField.h"
//#include <sofa/core/ForceField.inl>
#include <cassert>
#include <sofa/gl/template.h>
#include <iostream>
#include <sofa/core/visual/VisualParams.h>


namespace sofa
{

namespace component
{

namespace interactionforcefield
{

template<class DataTypes>
void RegistrationContactForceField<DataTypes>::clear(int reserve)
{
    prevContacts.swap(*contacts.beginEdit()); // save old contacts in prevContacts
    contacts.beginEdit()->clear();
    if (reserve)
        contacts.beginEdit()->reserve(reserve);
    contacts.endEdit();
}


template<class DataTypes>
void RegistrationContactForceField<DataTypes>::addContact(int m1, int m2, int index1, int index2, const Deriv& norm, Real dist, Real ks, Real mu_s, Real mu_v, int oldIndex)
{  
	int i = contacts.getValue().size();
	contacts.beginEdit()->resize(i+1);
	Contact& c = (*contacts.beginEdit())[i];
	c.m1 = m1;
	c.m2 = m2;
	c.index1 = index1;
	c.index2 = index2;
	c.norm = norm;
	c.dist = dist;
	c.ks = ks;
	c.mu_s = mu_s;
	c.mu_v = mu_v;
	c.pen = 0;
	if (oldIndex > 0 && oldIndex <= (int)prevContacts.size())
	{
	   c.age = prevContacts[oldIndex-1].age+1;
	}
	else
	{
	   c.age = 0;
        }
 	contacts.endEdit();
}

template<class DataTypes>
void RegistrationContactForceField<DataTypes>::addForce(const core::MechanicalParams* /*mparams*/ /* PARAMS FIRST */, DataVecDeriv& data_f1, DataVecDeriv& data_f2, const DataVecCoord& data_x1, const DataVecCoord& data_x2, const DataVecDeriv& /*data_v1*/, const DataVecDeriv& /*data_v2*/ )
{

	VecDeriv&       f1 = *data_f1.beginEdit();
	const VecCoord& x1 =  data_x1.getValue();
	//const VecDeriv& v1 =  data_v1.getValue();
	VecDeriv&       f2 = *data_f2.beginEdit();
	const VecCoord& x2 =  data_x2.getValue();
	//const VecDeriv& v2 =  data_v2.getValue();
    type::vector<Contact>& cc = *contacts.beginEdit();


	f1.resize(x1.size());
	f2.resize(x2.size());
       
    for (unsigned int i=0; i<cc.size(); i++)
	{
        Contact& c = cc[i];
		Coord u = x2[c.m2]-x1[c.m1];
		c.pen = c.dist - u*c.norm;
	//	if (c.pen > 0)
		{
			Real fN = c.ks * c.pen;
			Deriv force = -c.norm*fN;

            f1[c.m1]+=force;
            f2[c.m2]-=force;
		}
	}
 	contacts.endEdit();

	data_f1.endEdit();
	data_f2.endEdit();

}

template<class DataTypes>
void RegistrationContactForceField<DataTypes>::addDForce(const core::MechanicalParams* mparams /* PARAMS FIRST */, DataVecDeriv& data_df1, DataVecDeriv& data_df2, const DataVecDeriv& data_dx1, const DataVecDeriv& data_dx2)
{
	VecDeriv&        df1 = *data_df1.beginEdit();
	VecDeriv&        df2 = *data_df2.beginEdit();
	const VecDeriv&  dx1 =  data_dx1.getValue();
	const VecDeriv&  dx2 =  data_dx2.getValue();
    SReal kFactor = mparams->kFactor();
    const type::vector<Contact>& cc = contacts.getValue();

	df1.resize(dx1.size());
	df2.resize(dx2.size());
    for (unsigned int i=0; i<cc.size(); i++)
	{
        const Contact& c = cc[i];
		//if (c.pen > 0) // + dpen > 0)
		{
			Coord du = dx2[c.m2]-dx1[c.m1];
			Real dpen = - du*c.norm;
			//if (c.pen < 0) dpen += c.pen; // start penality at distance 0
			Real dfN = c.ks * dpen * (Real)kFactor;
			Deriv dforce = -c.norm*dfN;
			df1[c.m1]+=dforce;
			df2[c.m2]-=dforce;
		}
	}

	data_df1.endEdit();
	data_df2.endEdit();

}

template <class DataTypes>
    SReal RegistrationContactForceField<DataTypes>::getPotentialEnergy(const core::MechanicalParams* /* PARAMS FIRST */, const DataVecCoord&, const DataVecCoord& ) const
{
    msg_error()<<"RegistrationContactForceField::getPotentialEnergy-not-implemented !!!";
    return 0;
}

template<class DataTypes>
void RegistrationContactForceField<DataTypes>::draw(const core::visual::VisualParams* vparams)
{
#if REGISTRATION_HAVE_SOFA_GL == 1
	if (!((this->mstate1 == this->mstate2)?vparams->displayFlags().getShowForceFields():vparams->displayFlags().getShowInteractionForceFields())) return;
	const VecCoord& p1 = this->mstate1->read(core::vec_id::read_access::position)->getValue();
	const VecCoord& p2 = this->mstate2->read(core::vec_id::read_access::position)->getValue();
    const type::vector<Contact>& cc = contacts.getValue();

    glDisable(GL_LIGHTING);

    std::vector< type::Vec3 > points[4];

    for (unsigned int i=0; i<cc.size(); i++)
	{
        const Contact& c = cc[i];
		Real d = c.dist - (p2[c.m2]-p1[c.m1])*c.norm;
		if (c.age > 10) //c.spen > c.mu_s * c.ks * 0.99)
			if (d > 0)
			  {
				points[0].push_back(p1[c.m1]);
				points[0].push_back(p2[c.m2]);
			  }
			else
			  {
				points[1].push_back(p1[c.m1]);
				points[1].push_back(p2[c.m2]);
			  }
		else
			if (d > 0)
			  {
				points[2].push_back(p1[c.m1]);
				points[2].push_back(p2[c.m2]);
			  }
			else
			  {
				points[3].push_back(p1[c.m1]);
				points[3].push_back(p2[c.m2]);
			  }
	}

    vparams->drawTool()->drawLines(points[0], 1, type::RGBAColor::magenta());
    vparams->drawTool()->drawLines(points[1], 1, type::RGBAColor::cyan());
    vparams->drawTool()->drawLines(points[2], 1, type::RGBAColor::red());
    vparams->drawTool()->drawLines(points[3], 1, type::RGBAColor::green());

    std::vector< type::Vec3 > pointsN;
	if (vparams->displayFlags().getShowNormals())
	{
        for (unsigned int i=0; i<cc.size(); i++)
		{
            const Contact& c = cc[i];
			Coord p = p1[c.m1] - c.norm;
			pointsN.push_back(p1[c.m1]);
			pointsN.push_back(p);


			p = p2[c.m2] + c.norm;
			pointsN.push_back(p2[c.m2]);
			pointsN.push_back(p);
		}
        vparams->drawTool()->drawLines(pointsN, 1, type::RGBAColor::yellow());
	}
#endif // REGISTRATION_HAVE_SOFA_GL == 1
}


template<class DataTypes>
    void RegistrationContactForceField<DataTypes>::grabPoint(
    const core::behavior::MechanicalState<defaulttype::Vec3Types> *tool,
    const type::vector< unsigned int > &index,
    type::vector< std::pair< core::objectmodel::BaseObject*, type::Vec3f> > &result,
    type::vector< unsigned int > &triangle,
    type::vector< unsigned int > &index_point)
{
  if (static_cast< core::objectmodel::BaseObject *>(this->mstate1) == static_cast< const core::objectmodel::BaseObject *>(tool))
  {
    for (unsigned int i=0;i<contacts.getValue().size();i++)
    {
      for (unsigned int j=0;j<index.size();j++)
      {
	if (contacts.getValue()[i].m1  == (int)index[j])
	{
	  result.push_back(std::make_pair(static_cast< core::objectmodel::BaseObject *>(this),
			   this->mstate2->read(core::vec_id::read_access::position)->getValue()[contacts.getValue()[i].m2])
			  );
	  triangle.push_back(contacts.getValue()[i].index2);
	  index_point.push_back(index[j]);
	}
      }
    }
  }
  else if (static_cast< core::objectmodel::BaseObject *>(this->mstate2) == static_cast< const core::objectmodel::BaseObject *>(tool))
  {

    for (unsigned int i=0;i<contacts.getValue().size();i++)
    {
      for (unsigned int j=0;j<index.size();j++)
      {
	if (contacts.getValue()[i].m2  == (int)index[j])
	{
	  result.push_back(std::make_pair(static_cast< core::objectmodel::BaseObject *>(this),
			   this->mstate1->read(core::vec_id::read_access::position)->getValue()[contacts.getValue()[i].m1])
			  );

	  triangle.push_back(contacts.getValue()[i].index1);
	  index_point.push_back(index[j]);
	}
      }
    }
  }

}


} // namespace interactionforcefield

} // namespace component

} // namespace sofa

#endif  /* SOFA_COMPONENT_INTERACTIONFORCEFIELD_REGISTRATIONCONTACTFORCEFIELD_INL */
