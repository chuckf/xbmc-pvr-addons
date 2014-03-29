/*
 *      Copyright (C) 2014 Jean-Luc Barriere
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
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#ifndef MYTHWSRESPONSE_H
#define	MYTHWSRESPONSE_H

#include "mythwscontent.h"
#include "mythwsrequest.h"

#include <cstddef>  // for size_t
#include <string>

namespace Myth
{

  class TcpSocket;

  class WSResponse
  {
  public:
    WSResponse(const WSRequest& request);
    ~WSResponse();

    bool IsValid();
    size_t GetContentLength() const;
    size_t ReadContent(char *buf, size_t buflen);
    size_t GetConsumed() const;

  private:
    TcpSocket *m_socket;
    bool m_isValid;
    int m_statusCode;
    std::string m_serverInfo;
    std::string m_etag;
    CT_t m_contentType;
    size_t m_contentLength;
    size_t m_consumed;

    // prevent copy
    WSResponse(const WSResponse&);
    WSResponse& operator=(const WSResponse&);

    bool SendRequest(const WSRequest& request);
    bool GetResponse();
  };

}

#endif	/* MYTHWSRESPONSE_H */
