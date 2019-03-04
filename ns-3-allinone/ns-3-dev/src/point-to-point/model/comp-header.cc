/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/header.h"
#include "comp-header.h"
#include "ns3/log.h"
#include <iostream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CompHeader");

NS_OBJECT_ENSURE_REGISTERED (CompHeader);

/**
 * \ingroup network
 * A simple example of an Header implementation
 */
// class CompHeader : public Header
// {
// public:
//
//   CompHeader ();
//   virtual ~CompHeader ();
//
//   /**
//    * Set the header data.
//    * \param data The data.
//    */
//   void SetData (uint16_t data);
//   /**
//    * Get the header data.
//    * \return The data.
//    */
//   uint16_t GetData (void) const;
//
//   /**
//    * \brief Get the type ID.
//    * \return the object TypeId
//    */
//   static TypeId GetTypeId (void);
//   virtual TypeId GetInstanceTypeId (void) const;
//   virtual void Print (std::ostream &os) const;
//   virtual void Serialize (Buffer::Iterator start) const;
//   virtual uint32_t Deserialize (Buffer::Iterator start);
//   virtual uint32_t GetSerializedSize (void) const;
// private:
//   uint16_t m_data;  //!< Header data
// };

CompHeader::CompHeader ()
{
  // we must provide a public default constructor,
  // implicit or explicit, but never private.
}
CompHeader::~CompHeader ()
{
}

TypeId
CompHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CompHeader")
    .SetParent<Header> ()
    .AddConstructor<CompHeader> ()
  ;
  return tid;
}
TypeId
CompHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
CompHeader::Print (std::ostream &os) const
{
  // This method is invoked by the packet printing
  // routines to print the content of my header.
  //os << "data=" << m_data << std::endl;
  os << "data=" << m_data;
}
uint32_t
CompHeader::GetSerializedSize (void) const
{
  // we reserve 6 bytes for our header.
  return 6;
}
void
CompHeader::Serialize (Buffer::Iterator start) const
{
  // // we can serialize two bytes at the start of the buffer.
  // // we write them in network byte order.
  // start.WriteHtonU16 (m_data);
  // The 2 byte-constant
  start.WriteU8 (0xfe);
  start.WriteU8 (0xef);
  // The data.
  start.WriteHtonU32 (m_data);
}
uint32_t
CompHeader::Deserialize (Buffer::Iterator start)
{
  // // we can deserialize two bytes from the start of the buffer.
  // // we read them in network byte order and store them
  // // in host byte order.
  // m_data = start.ReadNtohU16 ();
  //
  // // we return the number of bytes effectively read.
  // return 2;

  uint8_t tmp;
  tmp = start.ReadU8 ();
  NS_ASSERT (tmp == 0xfe);
  tmp = start.ReadU8 ();
  NS_ASSERT (tmp == 0xef);
  m_data = start.ReadNtohU32 ();
  return 6; // the number of bytes consumed
}

void
CompHeader::SetData (uint16_t data)
{
  m_data = data;
}
uint16_t
CompHeader::GetData (void) const
{
  return m_data;
}

} //namespace ns3
