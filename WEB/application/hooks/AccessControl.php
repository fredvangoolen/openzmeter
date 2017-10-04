<?php

/*
 * Copyright (C) 2015 Pedro Sánchez Alguacil
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

class AccessControl {

    function checkLogin() {
        $this->CI = & get_instance();
        //echo $this->CI->session->logged;
        if($this->CI->router->class == 'dashboard' && $this->CI->router->method == 'liveData') return;
        
        if ($this->CI->session->logged == 1 || $this->CI->router->class == 'login') {
            if ($this->CI->session->logged == 1 && $this->CI->router->class == 'login' && $this->CI->router->method != 'out') {
                //echo "Pestes";
                //exit();
                redirect('');
            }
            //echo "Te dejo entrar";
//Logged
        } else {
            echo "No logueado";
            redirect('login');
        }
    }

}
