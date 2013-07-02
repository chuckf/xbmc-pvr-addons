/*
 *      Copyright (C) 2005-2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "MythFile.h"
#include "MythPointer.h"

using namespace ADDON;

MythFile::MythFile()
  : m_file_t(new MythPointer<cmyth_file_t>())
  , m_conn(MythConnection())
  , m_CutListSize(0)
{
}

MythFile::MythFile(cmyth_file_t myth_file, MythConnection conn)
  : m_file_t(new MythPointer<cmyth_file_t>())
  , m_conn(conn)
  , m_CutListSize(0)
{
  *m_file_t = myth_file;
}

bool MythFile::IsNull() const
{
  if (m_file_t == NULL)
    return true;
  return *m_file_t == NULL;
}

unsigned long long MythFile::Length()
{
  unsigned long long retval = 0;
  retval = cmyth_file_length(*m_file_t);
  return retval;
}

void MythFile::UpdateLength(unsigned long long length)
{
  int retval = 0;
  retval = cmyth_file_update_length(*m_file_t, length);
  (void)retval;
}

int MythFile::Read(void *buffer, unsigned int length)
{
  int bytesRead;

  if (m_CutListSize > 0)
  {
    while (m_Cut != NULL && this->Position() >= m_Cut->start)
    {
      XBMC->Log(LOG_DEBUG,"%s - Seek >>> cut: %llu", __FUNCTION__, m_Cut->end);
      if (this->SeekOverCut(m_Cut->end, WHENCE_SET) < 0)
        break;
    }
    if (m_Cut != NULL && (unsigned long long)(this->Position() + length) > m_Cut->start)
    {
      length = (unsigned int)(m_Cut->start - this->Position());
      XBMC->Log(LOG_DEBUG,"%s - Read (%u) --> cut: %llu", __FUNCTION__, length, m_Cut->start);
    }
  }
  bytesRead = cmyth_file_read(*m_file_t, static_cast< char * >(buffer), length);
  return bytesRead;
}

long long MythFile::Seek(long long offset, int whence)
{
  long long retval = 0;
  if (m_CutListSize > 0)
  {
    unsigned long long ppos = this->Position();
    retval = this->SeekOverCut(offset, whence);
    while (retval >= 0 && m_Cut != NULL && this->Position() > m_Cut->start)
    {
      if (this->Position() < ppos) {
        // On skip back, seek over:
        //   if ((offset = (long long)(this->Position() + m_Cut->start - m_Cut->end)) < 0)
        //     offset = 0;
        //   XBMC->Log(LOG_DEBUG,"%s - Seek <<< cut: %lld", __FUNCTION__, offset);
        // On skip back, ignore cut:
        XBMC->Log(LOG_DEBUG,"%s - Seek back! Ignore cut: %llu - %llu", __FUNCTION__, m_Cut->start, m_Cut->end);
        m_Cut->isIgnored = true;
        m_IgnoredCut = m_Cut;
        this->FindPinCutAround();
        break;
      }
      else
      {
        if ((offset = (long long)(this->Position() + m_Cut->end - m_Cut->start)) > (long long)this->Length())
          offset = (long long)this->Length();
        XBMC->Log(LOG_DEBUG,"%s - Seek >>> cut: %lld", __FUNCTION__, offset);
      }
      retval = this->SeekOverCut(offset, WHENCE_SET);
    }
  }
  else
    retval = cmyth_file_seek(*m_file_t, offset, whence);
  return retval;
}

unsigned long long MythFile::Position()
{
  unsigned long long retval = 0;
  retval = cmyth_file_position(*m_file_t);
  return retval;
}

void MythFile::ResetCutList()
{
  m_CutListSize = 0;
  m_Cut = NULL;
  m_IgnoredCut = NULL;
}

bool MythFile::AddCutEntry(unsigned long long start, unsigned long long end)
{
  if (m_CutListSize < FILE_CUT_LIST_SIZE)
  {
    m_CutList[m_CutListSize].start = start;
    m_CutList[m_CutListSize].end = end;
    m_CutList[m_CutListSize].isIgnored = false;
    PinCutAround(&m_CutList[m_CutListSize]);
    m_CutListSize++;
    return true;
  }
  else
    return false;
}

void MythFile::PinCutAround(FILE_CUT_ENTRY *cutEntry)
{
  if (!cutEntry->isIgnored)
  {
    if (cutEntry->end > this->Position() && (m_Cut == NULL || cutEntry->start < m_Cut->start))
    {
      m_Cut = cutEntry;
    }
  }
}

void MythFile::FindPinCutAround()
{
  m_Cut = NULL;
  for (int i = 0; i < m_CutListSize; i++)
    PinCutAround(&m_CutList[i]);
}

long long MythFile::SeekOverCut(long long offset, int whence)
{
  long long retval = 0;
  unsigned long long pos;
  if ((retval = cmyth_file_seek(*m_file_t, offset, whence)) >= 0)
  {
    pos = (unsigned long long)retval;
    if (m_IgnoredCut != NULL)
    {
      if (pos < m_IgnoredCut->start || pos >= m_IgnoredCut->end)
        m_IgnoredCut->isIgnored = false;
      else
        m_IgnoredCut->isIgnored = true;
    }
    this->FindPinCutAround();
  }
  return retval;
}
