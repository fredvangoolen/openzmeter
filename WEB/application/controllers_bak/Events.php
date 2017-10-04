<?php

defined('BASEPATH') OR exit('No direct script access allowed');

class Events extends CI_Controller {

    public function index() {
        redirect('events/itic');
    }

    public function itic() {
        $this->load->view('events/itic_view');
    }

    public function disturbances() {
        $this->load->view('events/disturbances_view');
    }

}
