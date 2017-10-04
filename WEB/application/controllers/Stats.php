<?php

defined('BASEPATH') OR exit('No direct script access allowed');

class Stats extends CI_Controller {

    public function index() {
        $data['energy'] = new stdClass();
        try {
            $query = $this->db->select('sampletime as time, active_energy as value')->where('sampletime >', strtotime('- 365 day') * 1000)->order_by('sampletime', 'desc')->get('agreg_10m');
            $res = $query->result();
        } catch (Exception $ex) {
            $res = array();
        }
//        if (count($res) > 0) {
        $data['energy'] = $res;



        $this->load->model('data_model');
        $data['main_values'] = $this->data_model->getMainValues();

        $this->load->model('event_model');
        $data['numEvents'] = $this->event_model->getNumUnreadEvents($this->session->eventTimestamp);

        $this->load->view('stats_view', $data);
    }

}
