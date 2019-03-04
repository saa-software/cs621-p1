/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef COMP_HEADER_H
#define COMP_HEADER_H

#include "ns3/header.h"


namespace ns3 {

/**
 * \ingroup packet
 *
 * \brief Protocol header serialization and deserialization.
 *
 * Every Protocol header which needs to be inserted or removed
 * from a Packet instance must derive from this base class and
 * implement the pure virtual methods defined here.
 *
 * Sample code which shows how to create a new type of Header, and how to use it,
 * is shown in the sample file samples/main-packet-header.cc
 */
class CompHeader : public Header
{
public:
  /**
  * Constuct CompHeader
  */
  CompHeader();

  virtual ~CompHeader ();
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Set the header data.
   * \param data The data.
   */
  void SetData (uint16_t data);
  /**
   * Get the header data.
   * \return The data.
   */
  uint16_t GetData (void) const;

  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual uint32_t GetSerializedSize (void) const;



private:
  uint16_t m_data;  //!< Header data
};
}

#endif /* HEADER_H */
