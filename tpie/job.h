// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2011, The TPIE development team
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

#ifndef __TPIE_JOB_MANAGER_H__
#define __TPIE_JOB_MANAGER_H__

///////////////////////////////////////////////////////////////////////////////
/// \file job.h Job class for job manager.
///////////////////////////////////////////////////////////////////////////////
#include <tpie/config.h>
#include <stddef.h>
#include <boost/thread.hpp>
#include <tpie/types.h>

namespace tpie {

class TPIE_PUBLIC job {

	enum job_state { job_idle, job_enqueued, job_running };

public:

	///////////////////////////////////////////////////////////////////////////
	/// \brief Default constructor.
	///////////////////////////////////////////////////////////////////////////
	job();

	///////////////////////////////////////////////////////////////////////////
	/// \brief Default destructor.
	///////////////////////////////////////////////////////////////////////////
	virtual ~job() {}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by the worker thread.
	///////////////////////////////////////////////////////////////////////////
	virtual void operator()() = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Wait for this job and its subjobs to complete.
	///////////////////////////////////////////////////////////////////////////
	void join();

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return true if this job and its subjobs are done.
	///////////////////////////////////////////////////////////////////////////
	bool is_done();

	///////////////////////////////////////////////////////////////////////////
	/// \brief Add this job to the job pool.
	///
	/// \param parent (optional) The parent job, or 0 if this is a root job.
	///////////////////////////////////////////////////////////////////////////
	void enqueue(job * parent = 0);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Run this job.
	///
	/// Invoke operator() and call done().
	///////////////////////////////////////////////////////////////////////////
	void run();

protected:

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called when this job and all subjobs are done.
	///////////////////////////////////////////////////////////////////////////
	virtual void on_done() {}

private:

	size_t m_dependencies;
	job * m_parent;
	job_state m_state;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Notified when this job and subjobs are done.
	///////////////////////////////////////////////////////////////////////////
	boost::condition_variable m_done;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called when this job or a subjob is done.
	///
	/// Decrement m_dependencies and call on_done() and notify
	/// waiters, if applicable.
	///////////////////////////////////////////////////////////////////////////
	void done();

	///////////////////////////////////////////////////////////////////////////
	/// The job manager needs to invoke run() on us.
	///////////////////////////////////////////////////////////////////////////
	friend class job_manager;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Return the number of job threads initialized by the job framework
/// in init_job().
///
/// When hardware concurrency is less than four, for instance on a dual-core
/// processor without hyperthreading, or a single-core processor, the number of
/// threads launched is equal to the number of cores. When hardware concurrency
/// is four or greater, the job framework spares one core for the user
/// interface.
///////////////////////////////////////////////////////////////////////////////
TPIE_PUBLIC memory_size_type default_worker_count();

///////////////////////////////////////////////////////////////////////////////
/// \internal \brief Used by tpie_init to initialize the job subsystem.
///////////////////////////////////////////////////////////////////////////////
TPIE_PUBLIC void init_job();

///////////////////////////////////////////////////////////////////////////////
/// \internal \brief Used by tpie_finish to deinitialize the job subsystem.
///////////////////////////////////////////////////////////////////////////////
TPIE_PUBLIC void finish_job();

} // namespace tpie

#endif
