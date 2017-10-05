<?php

defined('BASEPATH') OR exit('No direct script access allowed');

class Login extends CI_Controller {

    public function liveData() {
    $snd_msg = "VALUES|0";
    $rcv_msg = "";
    $timeout = 5;
    $socket = stream_socket_client('unix:///var/run/openZmeter.sock', $errorno, $errorstr, $timeout);
    stream_set_timeout($socket, $timeout);
    if(fwrite($socket, $snd_msg)) {
      do {
        $buff = @fread($socket, 8192);
        if(strlen($buff) == 0) break;
        echo $buff;
      } while (true);
    }
  }
  
    public function index() {
        $this->load->view('login_view');
    }

    public function in() {
        if ($this->input->is_ajax_request()) {
            $username = $this->input->post('username');
            $password = $this->input->post('password');
            $file = fopen("application/config/user.cfg", "r");
            $line = fgets($file);
            $parts = explode("|", $line);
            $hash = sha1($username . $this->config->item('encryption_key') . $password);
            if ($hash == $parts[1]) {
                $this->load->model('event_model');
                $this->session->eventTimestamp = $this->event_model->getWebEventTimestamp();
                $this->session->logged = TRUE;
                $this->session->username = $parts[0];
                echo json_encode(array('result' => TRUE));
            } else {
                echo json_encode(array('result' => FALSE));
            }
        }
    }

    public function out() {
        $this->session->sess_destroy();
        redirect('login');
    }

}
