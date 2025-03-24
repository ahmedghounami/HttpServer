/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsingBody.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 00:16:24 by hboudar           #+#    #+#             */
/*   Updated: 2025/03/20 00:18:04 by hboudar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../server.hpp"

bool takeBody_ChunkedFormData(client_info &client) {
  if (client.bodyReached == false || client.chunk.empty() == true)
    return false;
  std::cerr << "taking body[start]" << std::endl;

  // std::ofstream file(client.filename, std::ios::binary | std::ios::app);
  // file << client.chunk;
  // client.chunk.clear();
  // close();

  while (!client.chunk.empty()) {
    //step 1: read chunk size
    size_t pos = client.chunk.find("\r\n");
    if (pos == std::string::npos) {
      std::cerr << "ERROR: Invalid chunked format (no CRLF after size)" << std::endl;
      //respond and clear client;
      return false;
    }

    std::string chunkSizeStr = client.chunk.substr(0, pos);
    client.chunk.erase(0, pos + 2);

    // size_t chunkSize;
    // std::istringstream(chunkSizeStr) >> std::hex >> chunkSize;

    //step 2: check for final chunk
    }   

  std::cerr << "taking body[end]\n" << std::endl;
  return true;
}