/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hamza <hamza@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/09 22:31:40 by hboudar           #+#    #+#             */
/*   Updated: 2025/03/10 15:53:23 by hamza            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

int main(int argc, char const *argv[]) {
  try {
    // if (argc != 2)
    //   throw std::runtime_error("Usage: ./server <config_file.conf>");
    // int len = strlen(argv[1]);
    // if (len < 6 || argv[1][len - 1] != 'f' || argv[1][len - 2] != 'n' ||
    //     argv[1][len - 3] != 'o' || argv[1][len - 4] != 'c' ||
    //     argv[1][len - 5] != '.')
    //   throw std::runtime_error("Invalid config file");
    // std::ifstream file(argv[1]);
    // if (!file.is_open())
    //   throw std::runtime_error("Config file could not be opened");
    server obj;
    obj.listen_for_connections();
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }

  (void)argc;
  (void)argv;
  return 0;
}
