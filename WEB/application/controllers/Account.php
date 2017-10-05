<?php

defined('BASEPATH') OR exit('No direct script access allowed');

class Account extends CI_Controller {

    public function index() {
        $data = array();
        $file = fopen("application/config/user.cfg", "r");
        $line = fgets($file);
        $parts = explode("|", $line);
        $credentials = new stdClass();
        $credentials->user = $parts[0];
        $data['settings'] = array('credentials' => $credentials);

        $this->load->model('event_model');
        $data['numEvents'] = $this->event_model->getNumUnreadEvents($this->session->eventTimestamp);

        $this->load->view('account_view', $data);
    }

    public function ajaxSaveAccount() {
        if ($this->input->is_ajax_request()) {
            $credentials = $this->input->post('credentials');
            $hash = sha1($credentials['user'] . $this->config->item('encryption_key') . $credentials['pass']);
            $result = file_put_contents("application/config/user.cfg", $credentials['user'] . '|' . $hash, LOCK_EX);
            if ($result > 0) {
                $result = true;
            }
            echo json_encode(array('result' => $result));
        }
    }

}
