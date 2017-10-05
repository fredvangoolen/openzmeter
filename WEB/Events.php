<?php

defined('BASEPATH') OR exit('No direct script access allowed');

class Events extends CI_Controller {

    public function index() {
        redirect('events/itic');
    }

    public function itic() {
        $this->load->model('event_model');
        $data['events'] = $this->event_model->getEvents(strtotime('today - 365 day') * 1000, strtotime('today + 1 day') * 1000);

        $this->load->model('data_model');
        $data['main_values'] = $this->data_model->getMainValues();

        $data['numEvents'] = $this->event_model->getNumUnreadEvents($this->session->eventTimestamp);

        $this->load->view('events/itic_view', $data);
    }

    public function disturbances($timestamp = NULL) {
        $start = strtotime('today -1 day') * 1000;
        $end = strtotime('today + 1 day') * 1000;
        if ($timestamp != null) {
            $start = $timestamp - 60000;
            $end = $timestamp + 60000;
            $this->load->model('event_model');
            $data['timestamp'] = $timestamp;
        }
        $this->load->model('event_model');
        $data['events'] = $this->event_model->getEvents($start, $end, NULL, TRUE);

        $this->load->model('data_model');
        $data['main_values'] = $this->data_model->getMainValues();

        $data['numEvents'] = $this->event_model->getNumUnreadEvents($this->session->eventTimestamp);

        $this->load->view('events/disturbances_view', $data);
    }

    public function ajaxGetNumUnreadEvents() {
        $now = time();
        $startTime = $this->session->eventTimestamp;
        $this->load->model('event_model');
        try {
            $numEvents = $this->event_model->getNumUnreadEvents($startTime, NULL, 8);
        } catch (Exception $ex) {
            $numEvents = 0;
        }
        echo json_encode($numEvents);
    }

    public function ajaxGetLastEvents() {
        $now = time() * 1000;
        $startTime = $this->session->eventTimestamp;
        $this->load->model('event_model');
        try {
            $events = $this->event_model->getEvents(+$startTime * 1000, NULL, 8);
        } catch (Exception $ex) {
            $events = array();
        }
        $this->session->eventTimestamp = $now;
        $this->event_model->setWebEventTimestamp($now);
        echo json_encode(array('events' => $events, 'timestamp' => $startTime));
    }

    public function ajaxGetEventSamples() {
        if ($this->input->is_ajax_request()) {
            $this->load->model('event_model');
            $res = $this->event_model->getEventSamples($this->input->post('timestamp'));
            echo json_encode($res);
        }
    }

}
