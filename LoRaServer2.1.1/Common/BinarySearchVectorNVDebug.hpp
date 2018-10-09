/* � 2015 Semtech Corporation, 200 Flynn Road, Camarillo, CA 93012 U.S.A. All rights reserved.
The copyright of this file is owned by Semtech Corporation (Semtech). This is an unpublished work.  The content of this file must be used only for the purpose for which it was  supplied by Semtech or its distributors. The content of this file must not be copied or disclosed unless authorized in writing by Semtech.  */

#ifndef BINARY_SEARCH_VECTOR_NV_DEBUG_HPP
#define BINARY_SEARCH_VECTOR_NV_DEBUG_HPP

#include "General.h"
#include "BinarySearchVectorDebug.hpp"
#include "MutexDebug.hpp"
#include "ExceptionClass.hpp"
#include "Utilities.hpp"

#include <memory.h>
#include <time.h>

namespace BinarySearchVectorNV
{
	template <class ElementType, class IdType> class DatabaseListDebug
	{
	public:
		DatabaseListDebug() {}

		void Delete(IdType id) const;
		void DeleteAllElements() const;
	};

	template <class ElementType, class IdType> class ListDebug
	{
	protected:
		mutable MutexDebug									mutex;
		BinarySearchVectorDebug::List<ElementType, IdType>	memory;
		DatabaseListDebug<ElementType, IdType>				database;
		bool												deleteInternally;

	public:
		ListDebug(bool initialise)	//non ticked constructor
			: deleteInternally(false)
		{
			if (initialise)
				Initialise();
		}

		ListDebug(bool initialise, uint32 myListTickPeriod, uint32 myElementTickPeriod_ms, uint32 myElementTimeout_ms, bool myDeleteInternally)	//ticked constructor
			: memory(myListTickPeriod, myElementTickPeriod_ms, myElementTimeout_ms, false), deleteInternally(myDeleteInternally)
		{
			if (initialise)
				Initialise();
		}
		//lint -e{1551} Warning -- Function may throw exception '...'
		virtual ~ListDebug()	{DeleteAllElements(false);}

		void Initialise(); // must be written for each template instantiation

		void Add(ElementType const* element)
		{
			MutexHolderDebug holder(mutex,"Add");

			element->CreateNVRecord();

			memory.Add((BinarySearchVectorDebug::ElementTemplate<ElementType, IdType> *) const_cast<ElementType*>(element));
		}

		bool DeleteById(IdType id)	//returns false if not deleted
		{
			MutexHolderDebug holder(mutex,"Delete");
			database.Delete(id);
			return memory.DeleteById(id);
		}

		ElementType* GetByIndex(uint32 index) const	// returned element is always locked
		{return static_cast<ElementType*>(memory.GetByIndex(index));}
	
		ElementType* GetById(IdType id) const {return static_cast<ElementType*>(memory.GetById(id));}	// returned element is always locked

		bool Exists(IdType id) const {return memory.Exists(id);}

		IdType FindNextFreeId(IdType min) const {return memory.FindNextFreeId(min);}

		//tick should be called with a regular period of myListTickPeriod_ms
		//if the function returns with a value other than ~IdType(0), the function should be called again to ensure correct timing

		//If deleteInternally is true, deletes any timed out element.  Return value is always ~IdType(0)
		//If deleteInternally is false, returns the Id of an element that must be deleted.  
		IdType Tick()
		{
			IdType id;

			for (;;)
			{
				id = memory.Tick();

				if (id == ~IdType(0))
					break; //tick completed, no elements to delete

				//timed out element found
				if (deleteInternally)
					DeleteById(id);
				else
					break;	// returning id of element to be deleted
			}

			return id;
		}

		BinarySearchVectorDebug::IndexType Size() const {return BinarySearchVectorDebug::IndexType(memory.Size());}

		void DeleteAllElements(bool deleteFromNVStore = true)
		{
			MutexHolderDebug holder(mutex,"DeleteAllElements");

			if (deleteFromNVStore)
				database.DeleteAllElements();

			memory.DeleteAllElements();
		}
	};
}

#endif

