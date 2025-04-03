/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 15:36:48 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/03 16:02:46 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

class Client{
    private:
        int _listfd;
        int _clfd;
        int _state;
        Client obj(const Client& other) = delete;
        Client& operator=(const Client& other) = delete;
    public:
        Client();
        ~Client();
        int&    getFlag(void) const;
        void    setFlag(int newState);
};