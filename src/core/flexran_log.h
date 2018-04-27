/*
 * Copyright 2016-2018 FlexRAN Authors, Eurecom and The University of Edinburgh
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * For more information about Mosaic5G:  contact@mosaic-5g.io
 */

/*! \file    flexran_log.h
 *  \brief   definition of loggers
 *  \authors Xenofon Foukas, Robert Schmidt
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk, robert.schmidt@eurecom.fr
 */

#ifndef FLEXRAN_LOG_H_
#define FLEXRAN_LOG_H_

#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/helpers/exception.h"

namespace flexran_log = log4cxx;

namespace flexran {

  namespace core {

    static flexran_log::LoggerPtr core_logger(flexran_log::Logger::getLogger("FLEXRAN_RTC"));

    static flexran_log::LoggerPtr rib_logger(flexran_log::Logger::getLogger("RIB"));

    static flexran_log::LoggerPtr net_logger(flexran_log::Logger::getLogger("NET"));

    static flexran_log::LoggerPtr app_logger(flexran_log::Logger::getLogger("APP"));
    
  }
}

namespace flog {
  static flexran_log::LoggerPtr core = flexran::core::core_logger;
  static flexran_log::LoggerPtr net  = flexran::core::net_logger;
  static flexran_log::LoggerPtr rib  = flexran::core::rib_logger;
  static flexran_log::LoggerPtr app  = flexran::core::app_logger;
}

#endif
