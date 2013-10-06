
#include "CSP.h"
#include "..\source\RandomSequence.h"

#include <algorithm>

namespace Problems
{
	namespace BPP
	{

		Point& Point::operator =(const Point& rhs)
		{
			_x = rhs._x;
			_y = rhs._y;

			return *this;
		}

		Point& Point::operator +=(const Size& rhs)
		{
			_x += rhs.GetWidth();
			_y += rhs.GetHeight();

			return *this;
		}

		Point& Point::operator -=(const Size& rhs)
		{
			_x -= rhs.GetWidth();
			_y -= rhs.GetHeight();

			return *this;
		}

		int Size::FitFirst(Size& size) const
		{
			int fit = FitOriginal( size );
			if( fit < 0 )
			{
				Size rotated = size.GetRotated();
				fit = FitOriginal( rotated );

				if( fit >= 0 )
					size = rotated;
			}

			return fit;
		}

		int Size::FitBest(Size& size) const
		{
			int fit = FitOriginal( size );

			Size rotated = size.GetRotated();

			int rotatedFit = FitOriginal( rotated );
			if( rotatedFit >= 0 && ( fit < 0 || rotatedFit < fit ) )
			{
				size = rotated;
				fit = rotatedFit;
			}

			return fit;
		}

		Size& Size::operator +=(const Size& rhs)
		{
			_width += rhs._width;
			_height += rhs._height;

			return *this;
		}

		Size& Size::operator -=(const Size& rhs)
		{
			_width -= rhs._width;
			_height -= rhs._height;

			return *this;
		}

		Size& Size::operator =(const Size& rhs)
		{
			_width = rhs._width;
			_height = rhs._height;

			return *this;
		}

		void AddSlot(std::vector<Slot>& slots, const Slot& slot)
		{
			for( std::vector<Slot>::iterator it = slots.begin(); it != slots.end(); ++it )
			{
				if( it->Merge( slot ) )
					return;
			}

			slots.push_back( slot );
		}

		void Slot::Place(const Placement& placement, std::vector<Slot>& slots)
		{
			if( _area.IsOverlapping( placement.GetArea() ) )
			{
				int distance;
				const Rectangle& area = placement.GetArea();

				distance = area.GetPosition().GetX() - _area.GetPosition().GetX();
				if( distance > 0 )
					AddSlot( slots, Slot( _area.GetPosition(), Size( distance, _area.GetSize().GetHeight() ) ) );

				distance = _area.GetLimit().GetX() - area.GetLimit().GetX();
				if( distance > 0 )
					AddSlot( slots, Slot( Point( area.GetLimit().GetX(), _area.GetPosition().GetY() ), Size( distance, _area.GetSize().GetHeight() ) ) );

				distance = area.GetPosition().GetY() - _area.GetPosition().GetY();
				if( distance > 0 )
					AddSlot( slots, Slot( _area.GetPosition(), Size( _area.GetSize().GetWidth(), distance ) ) );

				distance = _area.GetLimit().GetY() - area.GetLimit().GetY();
				if( distance > 0 )
					AddSlot( slots, Slot( Point( _area.GetPosition().GetX(), area.GetLimit().GetY() ), Size( _area.GetSize().GetWidth(), distance ) ) );
			}
			else
				AddSlot( slots, *this );
		}

		bool Slot::Merge(const Slot& slot)
		{
			if( _area.GetSize().GetWidth() == slot._area.GetSize().GetWidth() && _area.GetPosition().GetX() == slot._area.GetPosition().GetX() )
			{
				if( ( slot._area.GetPosition().GetY() >= _area.GetPosition().GetY() && slot._area.GetPosition().GetY() <= _area.GetLimit().GetY() ) ||
					( slot._area.GetLimit().GetY() >= _area.GetPosition().GetY() && slot._area.GetLimit().GetY() <= _area.GetLimit().GetY() ) )
				{
					int y1 = std::min( slot._area.GetPosition().GetY(), _area.GetPosition().GetY() );
					int y2 = std::max( slot._area.GetLimit().GetY(), _area.GetLimit().GetY() );

					_area = Rectangle( Point( slot._area.GetPosition().GetX(), y1 ), Size( _area.GetSize().GetWidth(), y2 - y1 ) );
					return true;
				}
			}
			else if ( _area.GetSize().GetHeight() == slot._area.GetSize().GetHeight() && _area.GetPosition().GetY() == slot._area.GetPosition().GetY() )
			{
				if( ( slot._area.GetPosition().GetX() >= _area.GetPosition().GetX() && slot._area.GetPosition().GetX() <= _area.GetLimit().GetX() ) ||
					( slot._area.GetLimit().GetX() >= _area.GetPosition().GetX() && slot._area.GetLimit().GetX() <= _area.GetLimit().GetX() ) )
				{
					int x1 = std::min( slot._area.GetPosition().GetX(), _area.GetPosition().GetX() );
					int x2 = std::max( slot._area.GetLimit().GetX(), _area.GetLimit().GetX() );

					_area = Rectangle( Point( x1, slot._area.GetPosition().GetY() ), Size( x2 - x1, _area.GetSize().GetHeight() ) );
					return true;
				}
			}

			return false;
		}

		void Sheet::Clear()
		{
			_placements.clear();
			_slots.clear();
		}

		Sheet& Sheet::operator =(const Sheet& rhs)
		{
			_size = rhs._size;
			_placements = rhs._placements;
			_slots = rhs._slots;

			return *this;
		}

		void Sheet::AdjustSlots(const Placement& placement)
		{
			_placements.push_back( placement );

			std::vector<Slot> slots;
			for( std::vector<Slot>::iterator it = _slots.begin(); it != _slots.end(); ++it )
				it->Place( placement, slots );

			_slots = slots;
		}

		bool ClosesDistanceHeuristic(Placement& placement, Size orientation, bool rotation, const std::vector<Slot>& slots)
		{
			double distance = 0;
			bool placed = false;
			for( std::vector<Slot>::const_iterator it = slots.begin(); it != slots.end(); ++it )
			{
				if( it->GetArea().GetSize().FitBest( orientation ) >= 0 )
				{
					double d = std::sqrt( std::pow( it->GetArea().GetPosition().GetX(), 2 ) + std::pow( it->GetArea().GetPosition().GetY(), 2 ) );
					if( !placed || d < distance )
					{
						placement.SetArea( it->GetArea().GetPosition(), orientation );

						distance = d;
						placed = true;
					}
				}
			}

			return placed;
		}

		bool LowestPositionHeuristic(Placement& placement, Size orientation, bool rotation, const std::vector<Slot>& slots)
		{
			bool placed = false;
			for( std::vector<Slot>::const_iterator it = slots.begin(); it != slots.end(); ++it )
			{
				if( it->GetArea().GetSize().FitBest( orientation ) >= 0 )
				{
					if( !placed || ( it->GetArea().GetPosition().GetX() <= placement.GetArea().GetPosition().GetX() &&
						it->GetArea().GetPosition().GetY() <= placement.GetArea().GetPosition().GetY() ) )
					{
						placement.SetArea( it->GetArea().GetPosition(), orientation );

						placed = true;
					}
				}
			}

			return placed;
		}

		bool BestFitHeuristic(Placement& placement, Size orientation, bool rotation, const std::vector<Slot>& slots)
		{
			int fit = 0;
			bool placed = false;
			for( std::vector<Slot>::const_iterator it = slots.begin(); it != slots.end(); ++it )
			{
				int f = it->GetArea().GetSize().FitBest( orientation );
				if( f >= 0 )
				{
					if( !placed || f < fit )
					{
						placement.SetArea( it->GetArea().GetPosition(), orientation );

						fit = f;
						placed = true;
					}
				}
			}

			return placed;
		}

		void CspConfigBlock::SetItems(const Common::Data::GaSingleDimensionArray<Item>& items)
		{
			_items = items;
			_indices.SetSize( items.GetSize() );
			for( int i = _indices.GetSize() - 1; i >= 0; i-- )
				_indices[ i ] = i;
		}

		void CspChromosome::MutationEvent(GaChromosome::GaMuataionEvent e)
		{
			switch( e )
			{
			case Chromosome::GaChromosome::GAME_PREPARE: _backup = _sheet; break;
			case Chromosome::GaChromosome::GAME_ACCEPT: _backup.Clear(); break;
			case Chromosome::GaChromosome::GAME_REJECT: _sheet = _backup; break;
			}
		}

		Chromosome::GaChromosomePtr CspInitializator::operator ()(bool empty,
			const Chromosome::GaInitializatorParams& parameters,
			Common::Memory::GaSmartPtr<Chromosome::GaChromosomeConfigBlock> configBlock) const
		{
			CspChromosome* chromosome = new CspChromosome( configBlock );

			if( !empty )
			{
				CspConfigBlock& b = ( (CspConfigBlock&)( *configBlock ) );

				const Common::Data::GaSingleDimensionArray<Item>& items = b.GetItems();
				Common::Data::GaSingleDimensionArray<int> shuffled( b.GetIndices().GetArray(), items.GetSize() );
				Common::Random::GaShuffle( shuffled.GetArray(), items.GetSize() );

				Sheet& sheet = chromosome->GetSheet();
				for( int i = items.GetSize() - 1; i >= 0;  )
				{
					sheet.Place(LowestPositionHeuristic, items[ shuffled[ i ] ], items[ shuffled[ i ] ].GetSize(), true );
				}
			}

			return chromosome;
		}

		void CspFitnessOperation::operator ()(const CspFitnessOperation::GaObjectType& object,
			Fitness::GaFitness& fitness,
			const Fitness::GaFitnessOperationParams& operationParams) const
		{
		}

		void CspCrossoverOperation::operator ()(Chromosome::GaCrossoverBuffer& crossoverBuffer,
			const Chromosome::GaCrossoverParams& parameters) const
		{
		}

		void CspMutationOperation::operator ()(Chromosome::GaChromosome& chromosome,
			const Chromosome::GaMutationParams& parameters) const
		{
		}

	}
}