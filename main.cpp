/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/07 14:41:19 by hboudar           #+#    #+#             */
/*   Updated: 2025/03/07 15:03:24 by hboudar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

int main() {
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    std::cerr << "Socket creation failed!" << std::endl;
    return 1;
  }

  struct sockaddr_in address;
  std::memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    std::cerr << "Bind failed!" << std::endl;
    close(server_fd);
    return 1;
  }

  if (listen(server_fd, 5) < 0) {
    std::cerr << "Listen failed!" << std::endl;
    close(server_fd);
    return 1;
  }

  std::cout << "Server is alive on port " << PORT << "..." << std::endl;

  close(server_fd);
  return 0;
}