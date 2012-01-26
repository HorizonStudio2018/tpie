// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

#ifndef _INTERNAL_SORT_H
#define _INTERNAL_SORT_H

///////////////////////////////////////////////////////////////////////////
/// \file internal_sort.h
/// Provides base class Internal_Sorter_Base for internal sorter objects and
/// two subclass implementations Internal_Sorter_Op and Internal_Sorter_Obj.
/// Both implementations rely on quicksort variants quick_sort_op() and 
/// quick_sort_obj(), resp.
///////////////////////////////////////////////////////////////////////////

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>
#include <tpie/parallel_sort.h>
#include <tpie/comparator.h> //to convert TPIE comparisons to STL
#include <tpie/fractional_progress.h>
#include <tpie/memory.h>
#include <tpie/tpie_assert.h>

namespace tpie {
namespace ami {

//A simple class that facilitates doing key sorting followed 
//by in-memory permuting to sort items in-memory. This is 
//particularly useful when key size is much smaller than 
//item size. Note that using this requires that the class Key
//have the comparison operators defined appropriately.
template<class Key>
class qsort_item {
public:
	Key keyval;
	TPIE_OS_SIZE_T source;
	
	friend int operator==(const qsort_item &x, const qsort_item &y)
		{return  (x.keyval ==  y.keyval);}
	
	friend int operator!=(const qsort_item &x, const qsort_item &y)
		{return  (x.keyval !=  y.keyval);}    
	
	friend int operator<=(const qsort_item &x, const qsort_item &y)
		{return  (x.keyval <=  y.keyval);}
	
	friend int operator>=(const qsort_item &x, const qsort_item &y)
		{return  (x.keyval >=  y.keyval);}
	
	friend int operator<(const qsort_item &x, const qsort_item &y)
		{return  (x.keyval <  y.keyval);}
	
	friend int operator>(const qsort_item &x, const qsort_item &y)
		{return  (x.keyval >  y.keyval);}
};


///////////////////////////////////////////////////////////////////////////
/// The base class for internal sorters. 
/// This class does not have a sort() function, so it cannot be used directly.
///////////////////////////////////////////////////////////////////////////
template<class T>
class Internal_Sorter_Base {
protected:
	/** Array that holds items to be sorted */
	array<T> ItemArray;        
	/** length of ItemArray */
	TPIE_OS_SIZE_T len;  

public:
	///////////////////////////////////////////////////////////////////////////
	///  Empty constructor.
	///////////////////////////////////////////////////////////////////////////
	Internal_Sorter_Base(void): len(0) {
		//  No code in this constructor.
	};
	
	///////////////////////////////////////////////////////////////////////////
	/// Allocate ItemArray as array that can hold \p nItems.
	///////////////////////////////////////////////////////////////////////////
	void allocate(TPIE_OS_SIZE_T nItems);
	    
	///////////////////////////////////////////////////////////////////////////
	/// Clean up internal array ItemArray.
	///////////////////////////////////////////////////////////////////////////
	void deallocate(void); 
	    
	///////////////////////////////////////////////////////////////////////////
	/// Returns maximum number of items that can be sorted using \p memSize bytes.
	///////////////////////////////////////////////////////////////////////////
	TPIE_OS_SIZE_T MaxItemCount(TPIE_OS_SIZE_T memSize);
	
	///////////////////////////////////////////////////////////////////////////
	/// Returns memory usage in bytes per sort item.
	///////////////////////////////////////////////////////////////////////////
	TPIE_OS_SIZE_T space_per_item();
	    
	///////////////////////////////////////////////////////////////////////////
	/// Returns fixed memory usage overhead in bytes per class instantiation.
	///////////////////////////////////////////////////////////////////////////
	TPIE_OS_SIZE_T space_overhead();
	
private:
	// Prohibit these
	Internal_Sorter_Base(const Internal_Sorter_Base<T>& other);
	Internal_Sorter_Base<T> operator=(const Internal_Sorter_Base<T>& other);
};
	
template<class T>
inline void Internal_Sorter_Base<T>::allocate(TPIE_OS_SIZE_T nitems) {
	len=nitems;
	ItemArray.resize(len);
}

template<class T>
inline void Internal_Sorter_Base<T>::deallocate(void) {
	ItemArray.resize(0);
	len=0;
}

template<class T>
inline TPIE_OS_SIZE_T Internal_Sorter_Base<T>::MaxItemCount(TPIE_OS_SIZE_T memSize) {
	//Space available for items
	TPIE_OS_SIZE_T memAvail=memSize-space_overhead();
	
	if (memAvail < space_per_item()) return 0; 
	return memAvail/space_per_item(); 
}

	
template<class T>
inline TPIE_OS_SIZE_T Internal_Sorter_Base<T>::space_overhead(void) {
	// Space usage independent of space_per_item
	// accounts MM_manager space overhead on "new" call
	return 0;
}

template<class T>
inline TPIE_OS_SIZE_T Internal_Sorter_Base<T>::space_per_item(void) {
	return sizeof(T);
}
	
///////////////////////////////////////////////////////////////////////////
/// Comparision operator based Internal_Sorter_base subclass implementation; uses 
/// quick_sort_op().
///////////////////////////////////////////////////////////////////////////
template<class T>
class Internal_Sorter_Op: public Internal_Sorter_Base<T>{
protected:
	using Internal_Sorter_Base<T>::len;
	using Internal_Sorter_Base<T>::ItemArray;
	    
public:
	//  Constructor/Destructor
	Internal_Sorter_Op() {
		//  No code in this constructor.
	};
	
	~Internal_Sorter_Op() {
		//  No code in this destructor.
	};
	
	using Internal_Sorter_Base<T>::space_overhead;
	
	//Sort nItems from input stream and write to output stream
	void sort(file_stream<T>* InStr, file_stream<T>* OutStr, TPIE_OS_SIZE_T nItems, progress_indicator_base * pi=0);
	    
private:
	// Prohibit these
	Internal_Sorter_Op(const Internal_Sorter_Op<T>& other);
	Internal_Sorter_Op<T> operator=(const Internal_Sorter_Op<T>& other);
};
	
///////////////////////////////////////////////////////////////////////////
/// Reads nItems sequentially from InStr, starting at the current file
/// position; writes the sorted output to OutStr, starting from the current
/// file position.
///////////////////////////////////////////////////////////////////////////
template<class T>
void Internal_Sorter_Op<T>::sort(file_stream<T>* InStr, 
								 file_stream<T>* OutStr, 
								 TPIE_OS_SIZE_T nItems, 
								 progress_indicator_base * pi){
	TPIE_OS_SIZE_T i = 0;
	// make sure we called allocate earlier
	if (ItemArray.size() == 0) throw stream_exception("NULL_POINTER");

	
	tp_assert ( nItems <= len, "Internal buffer overfull (nItems > len)");

	fractional_progress fp(pi);
	fp.id() << __FILE__ << __FUNCTION__ << typeid(T);
	fractional_subindicator read_progress(fp, "read", TPIE_FSI, nItems, "Reading");
	fractional_subindicator sort_progress(fp, "sort", TPIE_FSI, nItems, "Sorting");
	fractional_subindicator write_progress(fp, "write", TPIE_FSI, nItems, "Writing");
	fp.init();

	read_progress.init(nItems);
	// Read a memory load out of the input stream one item at a time,
	for (i = 0; i < nItems; i++) {
		ItemArray[i] = InStr->read();
		read_progress.step();
	}
	read_progress.done();

	//Sort the array.
	parallel_sort<true>(ItemArray.begin(), ItemArray.begin()+nItems, sort_progress, std::less<T>());

	if(InStr==OutStr){ //Do the right thing if we are doing 2x sort
		//Internal sort objects should probably be re-written so that
		//the interface is cleaner and they don't have to worry about I/O
		InStr->truncate(0); //delete original items
		InStr->seek(0); //rewind
	}
	
	//  Write sorted array to OutStr
	write_progress.init(nItems);
	for (i = 0; i < nItems; i++) {
		OutStr->write(ItemArray[i]);
		write_progress.step();
	}
	write_progress.done();
	fp.done();
}

///////////////////////////////////////////////////////////////////////////
/// Comparision object based Internal_Sorter_base subclass implementation; uses 
/// quick_sort_obj().
///////////////////////////////////////////////////////////////////////////
template<class T, class CMPR>
class Internal_Sorter_Obj: public Internal_Sorter_Base<T>{
protected:
	using Internal_Sorter_Base<T>::ItemArray;
	using Internal_Sorter_Base<T>::len;
	/** Comparison object used for sorting */
	CMPR *cmp_o;
	
public:
	///////////////////////////////////////////////////////////////////////////
	/// Empty constructor.
	///////////////////////////////////////////////////////////////////////////
	Internal_Sorter_Obj(CMPR* cmp) :cmp_o(cmp) {};
	
	///////////////////////////////////////////////////////////////////////////
	/// Empty destructor.
	///////////////////////////////////////////////////////////////////////////
	~Internal_Sorter_Obj(){};
	
	using Internal_Sorter_Base<T>::space_overhead;
	
	//Sort nItems from input stream and write to output stream
	void sort(file_stream<T>* InStr, file_stream<T>* OutStr, TPIE_OS_SIZE_T nItems, 
			  progress_indicator_base * pi=0);
	
private:
	// Prohibit these
	Internal_Sorter_Obj(const Internal_Sorter_Obj<T,CMPR>& other);
	Internal_Sorter_Obj<T,CMPR> operator=(const Internal_Sorter_Obj<T,CMPR>& other);
};

///////////////////////////////////////////////////////////////////////////
/// Reads nItems sequentially from InStr, starting at the current file
/// position; writes the sorted output to OutStr, starting from the current
/// file position.
///////////////////////////////////////////////////////////////////////////
template<class T, class CMPR>
void Internal_Sorter_Obj<T, CMPR>::sort(file_stream<T>* InStr, 
										file_stream<T>* OutStr, 
										TPIE_OS_SIZE_T nItems,
										progress_indicator_base * pi) {
	tp_assert ( nItems <= len, "Internal buffer overfull (nItems > len)");

	
	TPIE_OS_SIZE_T i = 0;
	
	//make sure we called allocate earlier
	if (ItemArray.size() == 0) throw stream_exception("NULL_POINTER");
	    
	tp_assert ( nItems <= len, "Internal buffer overfull (nItems > len)");

	fractional_progress fp(pi);
	fp.id() << __FILE__ << __FUNCTION__ << typeid(T) << typeid(CMPR);
	fractional_subindicator read_progress(fp, "read", TPIE_FSI, nItems, "Reading");
	fractional_subindicator sort_progress(fp, "sort", TPIE_FSI, nItems, "Sorting");
	fractional_subindicator write_progress(fp, "write", TPIE_FSI, nItems, "Writing");
	fp.init();

	read_progress.init(nItems);
	// Read a memory load out of the input stream one item at a time,
	for (i = 0; i < nItems; i++) {
		ItemArray[i] = InStr->read();
		read_progress.step();
	}
	read_progress.done();

	//Sort the array.
	tpie::parallel_sort<true>(ItemArray.begin(), ItemArray.begin()+nItems, sort_progress, TPIE2STL_cmp<T,CMPR>(cmp_o));
	if (InStr==OutStr) { //Do the right thing if we are doing 2x sort
		//Internal sort objects should probably be re-written so that
		//the interface is cleaner and they don't have to worry about I/O
		InStr->truncate(0); //delete original items
		InStr->seek(0); //rewind
	}

	write_progress.init(nItems);
	//Write sorted array to OutStr
	for (i = 0; i < nItems; i++) {
		OutStr->write(ItemArray[i]);
		write_progress.step();
	}
	write_progress.done();
	fp.done();
}

////////////////////////////////////////////////////////////////////////
/// Key + Object based Internal Sorter; used by key_sort() routines.
////////////////////////////////////////////////////////////////////////
template<class T, class KEY, class CMPR>
class Internal_Sorter_KObj : public Internal_Sorter_Base<T> {
protected:
	/** Array that holds original items */
	array<T> ItemArray;

	/** Holds keys to be sorted */
	array<qsort_item<KEY> > sortItemArray;
	/** Copy,compare keys */ 
	CMPR *UsrObject;              
	/** length of ItemArray */
	TPIE_OS_SIZE_T len;
	
public:
	///////////////////////////////////////////////////////////////////////////
	///  Empty constructor.
	///////////////////////////////////////////////////////////////////////////
	Internal_Sorter_KObj(CMPR* cmp):  UsrObject(cmp), len(0) {
		//  No code in this constructor.
	}
	
	//////////////////////////////////////////////////////////////////////////
	///  Destructor calling deallocate.
	//////////////////////////////////////////////////////////////////////////
	~Internal_Sorter_KObj(void); //Destructor
    
	//////////////////////////////////////////////////////////////////////////
	/// Allocate array that can hold nItems.
	//////////////////////////////////////////////////////////////////////////
	void allocate(TPIE_OS_SIZE_T nItems);
	
	//////////////////////////////////////////////////////////////////////////
	/// Sort nItems from input stream and write to output stream.
	//////////////////////////////////////////////////////////////////////////
	void sort(file_stream<T>* InStr, file_stream<T>* OutStr, TPIE_OS_SIZE_T nItems, progress_indicator_base * pi=0);
	
	//////////////////////////////////////////////////////////////////////////
	/// Clean up internal array.
	//////////////////////////////////////////////////////////////////////////
	void deallocate(void); 
	
	//////////////////////////////////////////////////////////////////////////
	/// Returns maximum number of items that can be sorted using \p memSize bytes.
	//////////////////////////////////////////////////////////////////////////
	TPIE_OS_SIZE_T MaxItemCount(TPIE_OS_SIZE_T memSize);
	
	//////////////////////////////////////////////////////////////////////////
	/// Returns memory usage in bytes per sort item.
	//////////////////////////////////////////////////////////////////////////
	TPIE_OS_SIZE_T space_per_item();
	    
	//////////////////////////////////////////////////////////////////////////
	/// Returns fixed memory usage overhead in bytes per class instantiation.
	//////////////////////////////////////////////////////////////////////////
	TPIE_OS_SIZE_T space_overhead();
	
private:
	// Prohibit these
	Internal_Sorter_KObj(const Internal_Sorter_KObj<T,KEY,CMPR>& other);
	Internal_Sorter_KObj<T,KEY,CMPR> operator=(const Internal_Sorter_KObj<T,KEY,CMPR>& other);
};

template<class T, class KEY, class CMPR>
Internal_Sorter_KObj<T, KEY, CMPR>::~Internal_Sorter_KObj(void){
	//  In case someone forgot to call deallocate()	    
	ItemArray.resize(0);
	sortItemArray.resize(0);
}

template<class T, class KEY, class CMPR>
inline void Internal_Sorter_KObj<T, KEY, CMPR>::allocate(TPIE_OS_SIZE_T nitems){
	len=nitems;
	ItemArray.resize(len);
	sortItemArray.resize(len);
}

///////////////////////////////////////////////////////////////////////////
/// A helper class to quick sort qsort_item types
/// given a comparison object for comparing keys.
///////////////////////////////////////////////////////////////////////////
template<class KEY, class KCMP>
class QsortKeyCmp {
private:
	// Prohibit these.
	QsortKeyCmp(const QsortKeyCmp<KEY,KCMP>& other);
	QsortKeyCmp<KEY,KCMP>& operator=(const QsortKeyCmp<KEY,KCMP>& other);
	
	KCMP *isLess; //Class with function compare that compares 2 keys    
	
public:
      ///////////////////////////////////////////////////////////////////////////
      ///  Constructor.
      ///////////////////////////////////////////////////////////////////////////
      QsortKeyCmp(KCMP* kcmp) : isLess(kcmp) { };

      ///////////////////////////////////////////////////////////////////////////
      /// Comparision method;
      /// returns -1, 0, or +1 to indicate that <tt>left<right</tt>, <tt>left==right</tt>, or
      /// <tt>left>right</tt> respectively.
      ///////////////////////////////////////////////////////////////////////////
	inline int compare(const qsort_item<KEY>& left,
					   const qsort_item<KEY>& right){
		return isLess->compare(left.keyval, right.keyval);
	}
};

template<class T, class KEY, class CMPR>
inline void Internal_Sorter_KObj<T, KEY, CMPR>::sort(file_stream<T>* InStr,
													 file_stream<T>* OutStr, TPIE_OS_SIZE_T nItems,
													 progress_indicator_base * pi) {
	
	TPIE_OS_SIZE_T i = 0;

	// Make sure we called allocate earlier
	if (ItemArray.size() == 0 || sortItemArray.size() == 0) {
		throw stream_exception("NULL_POINTER");
	}
	
	tp_assert ( nItems <= len, "Internal buffer overfull (nItems > len)");

	fractional_progress fp(pi);
	fp.id() << __FILE__ << __FUNCTION__ << typeid(T) << typeid(KEY) <<  typeid(CMPR);
	fractional_subindicator read_progress(fp, "read", TPIE_FSI, nItems, "Reading");
	fractional_subindicator sort_progress(fp, "sort", TPIE_FSI, nItems, "Sorting");
	fractional_subindicator write_progress(fp, "write", TPIE_FSI, nItems, "Writing");
	fp.init();

	read_progress.init(nItems);
	// Read a memory load out of the input stream one item at a time,
	for (i = 0; i < nItems; i++) {
		ItemArray[i] = InStr->read();
		sortItemArray[i].source=i;
		read_progress.step();
	}
	read_progress.done();
	
	//Sort the array.
	QsortKeyCmp<KEY, CMPR> kc(UsrObject);
	TPIE2STL_cmp<qsort_item<KEY>,QsortKeyCmp<KEY,CMPR> > stlcomp(&kc);

	parallel_sort<true>(sortItemArray.begin(), sortItemArray.begin()+nItems, sort_progress, std::less<qsort_item<KEY> >());
	if (InStr==OutStr) { //Do the right thing if we are doing 2x sort
		//Internal sort objects should probably be re-written so that
		//the interface is cleaner and they don't have to worry about I/O
		InStr->truncate(0); //delete original items
		InStr->seek(0); //rewind
	}

	write_progress.init(nItems);
	//Write sorted array to OutStr
	for (i = 0; i < nItems; i++) {
		OutStr->write(ItemArray[sortItemArray[i].source]);
		write_progress.step();
	}
	write_progress.done();
	fp.done();
}

template<class T, class KEY, class CMPR>
inline void Internal_Sorter_KObj<T, KEY, CMPR>::deallocate(void) {
	
	len=0;
	ItemArray.resize(0);
	sortItemArray.resize(0);
}

template<class T, class KEY, class CMPR>
inline TPIE_OS_SIZE_T Internal_Sorter_KObj<T, KEY, CMPR>::MaxItemCount(TPIE_OS_SIZE_T memSize) {
	
	//Space available for items
	TPIE_OS_SIZE_T memAvail=memSize-space_overhead();
	
	if (memAvail < space_per_item()) return 0; 
	else return memAvail/space_per_item(); 
}
	
	
template<class T, class KEY, class CMPR>
inline TPIE_OS_SIZE_T Internal_Sorter_KObj<T, KEY, CMPR>::space_overhead(void) { 
	// Space usage independent of space_per_item
	// accounts MM_manager space overhead on "new" call
	return 0;
}

template<class T, class KEY, class CMPR>
inline TPIE_OS_SIZE_T Internal_Sorter_KObj<T, KEY, CMPR>::space_per_item(void) {
	return sizeof(T) + sizeof(qsort_item<KEY>);
}


}  //  ami namespace
}  //  tpie namespace
#endif // _INTERNAL_SORT_H 














































