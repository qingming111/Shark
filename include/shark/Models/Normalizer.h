/*!
 * 
 *
 * \brief       Model for scaling and translation of data vectors.
 * 
 * 
 *
 * \author      T. Glasmachers
 * \date        2013
 *
 *
 * \par Copyright 1995-2017 Shark Development Team
 * 
 * <BR><HR>
 * This file is part of Shark.
 * <http://shark-ml.org/>
 * 
 * Shark is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published 
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Shark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with Shark.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef SHARK_MODELS_NORMALIZER_H
#define SHARK_MODELS_NORMALIZER_H

#include <shark/Models/AbstractModel.h>
#include <shark/LinAlg/Base.h>


namespace shark {


///
/// \brief "Diagonal" linear model for data normalization.
///
/// \par
/// The Normalizer is a restricted and often more efficient variant of
/// the LinearModel class. It restricts the linear model in two respects:
/// (1) input and output dimension must agree,
/// (2) computations are independent for each component.
/// This is useful mostly for data normalization (therefore the name).
/// The model's operation is of the form \f$ x \mapsto A x + b \f$ where
/// A is a diagonal matrix. This reduces memory requirements to linear,
/// which is why there is no sparse version of this model (as opposed to
/// the more general linear model). Also, the addition of b is optional.
///
template <class DataType = RealVector>
class Normalizer : public AbstractModel<DataType, DataType>
{
public:
	typedef AbstractModel<DataType, DataType> base_type;
	typedef Normalizer<DataType> self_type;

	typedef typename base_type::BatchInputType BatchInputType;
	typedef typename base_type::BatchOutputType BatchOutputType;

	/// Constructor of an invalid model; use setStructure later
	Normalizer()
	{ }

	/// copy constructor
	Normalizer(const self_type& model)
	: m_A(model.m_A)
	, m_b(model.m_b)
	, m_hasOffset(model.m_hasOffset)
	{ }

	/// Construction from dimension
	Normalizer(std::size_t dimension, bool hasOffset = false)
	: m_A(dimension, dimension)
	, m_b(dimension)
	, m_hasOffset(hasOffset)
	{ }

	/// Construction from matrix
	Normalizer(RealVector diagonal)
	: m_A(diagonal)
	, m_hasOffset(false)
	{ }

	/// Construction from matrix and vector
	Normalizer(RealVector diagonal, RealVector vector)
	: m_A(diagonal)
	, m_b(vector)
	, m_hasOffset(true)
	{ }


	/// \brief From INameable: return the class name.
	std::string name() const
	{ return "Normalizer"; }

	/// swap
	friend void swap(const Normalizer& model1, const Normalizer& model2){
		std::swap(model1.m_A, model2.m_A);
		std::swap(model1.m_b, model2.m_b);
		std::swap(model1.m_hasOffset, model2.m_hasOffset);
	}

	/// assignment operator
	const self_type operator = (const self_type& model){
		m_A = model.m_A;
		m_b = model.m_b;
		m_hasOffset = model.m_hasOffset;
	}

	/// derivative storage object (empty for this model)
	boost::shared_ptr<State> createState() const
	{
		return boost::shared_ptr<State>(new EmptyState());
	}
	
	/// check for the presence of an offset term
	bool hasOffset() const{
		return m_hasOffset;
	}

	/// return the diagonal of the matrix
	RealVector const& diagonal() const{
		return m_A;
	}

	/// return the offset vector
	RealVector const& offset() const{
		return m_b;
	}

	/// obtain the input dimension
	std::size_t inputSize() const{
		return m_A.size();
	}

	/// obtain the output dimension
	std::size_t outputSize() const{
		return m_A.size();
	}

	/// obtain the parameter vector
	RealVector parameterVector() const{
		if (hasOffset())
			return m_A | m_b;
		else
			return m_A;
	}

	/// overwrite the parameter vector
	void setParameterVector(RealVector const& newParameters){
		SIZE_CHECK(newParameters.size() == numberOfParameters());
		std::size_t dim = m_A.size();
		noalias(m_A) = subrange(newParameters,0,dim);
		if (hasOffset()){
			noalias(m_b) = subrange(newParameters,dim, 2*dim);
		}
	}

	/// return the number of parameter
	std::size_t numberOfParameters() const{
		return (m_hasOffset) ? m_A.size() + m_b.size() : m_A.size();
	}

	/// overwrite structure and parameters
	void setStructure(RealVector const& diagonal){
		m_A = diagonal;
		m_hasOffset = false;
	}

	/// overwrite structure and parameters
	void setStructure(std::size_t dimension, bool hasOffset = false){
		m_A.resize(dimension);
		m_hasOffset = hasOffset;
		if (hasOffset) m_b.resize(dimension);
	}

	/// overwrite structure and parameters
	void setStructure(RealVector const& diagonal, RealVector const& offset){
		m_A = diagonal;
		m_b = offset;
		m_hasOffset = true;
	}

	using base_type::eval;

	/// \brief Evaluate the model: output = matrix * input + offset.
	void eval(BatchInputType const& input, BatchOutputType& output) const{
		SIZE_CHECK(input.size1() == m_A.size());
		output.resize(input.size1(), input.size2());
		noalias(output) = input * repeat(m_A,input.size1());
		if (hasOffset())
		{
			noalias(output) += repeat(m_b,input.size1());
		}
	}

	/// \brief Evaluate the model: output = matrix * input + offset.
	void eval(BatchInputType const& input, BatchOutputType& output, State& state) const{
		eval(input, output);
	}

	/// from ISerializable
	void read(InArchive& archive){
		archive & m_A;
		archive & m_b;
		archive & m_hasOffset;
	}

	/// from ISerializable
	void write(OutArchive& archive) const{
		archive & m_A;
		archive & m_b;
		archive & m_hasOffset;
	}

protected:
	RealVector m_A;                ///< matrix A (see class documentation)
	RealVector m_b;                        ///< vector b (see class documentation)
	bool m_hasOffset;                      ///< if true: add offset therm b; if false: don't.
};


}
#endif
