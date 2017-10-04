<?php

defined('BASEPATH') OR exit('No direct script access allowed');

class Dashboard extends CI_Controller {

    public function index() {
        $this->influxdb->setTimePrecision('ms');
        $db = $this->influxdb->getDatabase();

        $data['energy'] = new stdClass();
        $res = $db->query("SELECT time, SUM(val) FROM 1h.energy.w where time > now() - 365d GROUP BY time(1d);");
        $data['energy']->one_day = $res['1h.energy.w'];

        $res = $db->query("SELECT SUM(val) FROM 1h.energy.w where time > " . strtotime(date('Y-m-01')) . "s");
        if (count($res) > 0) {
            $data['energy']->this_month = $res['1h.energy.w'][0]->sum / 1000;
        } else {
            $data['energy']->this_month = 0;
        }

        $m = date('n');
        $lastmonth_start = mktime(1, 1, 1, $m - 1, 1, date('Y'));
        $res = $db->query("SELECT SUM(val) FROM 1h.energy.w where time > " . $lastmonth_start . "s AND time < " . strtotime(date('Y-m-01')) . 's');
        if (count($res) > 0) {
            $data['energy']->last_month = $res['1h.energy.w'][0]->sum / 1000;
        } else {
            $data['energy']->last_month = 0;
        }

        $res = $db->query("SELECT time, MEAN(value) FROM 10c.voltage.rms, 10c.current.rms,  10c.power.p where time > now() - 1m GROUP BY time(3s);");
        $data['voltage'] = new stdClass();
        $data['voltage']->last_values = isset($res['10c.voltage.rms']) ? $res['10c.voltage.rms'] : array();
        $data['current'] = new stdClass();
        $data['current']->last_values = isset($res['10c.current.rms']) ? $res['10c.current.rms'] : array();
        $data['power'] = new stdClass();
        $data['power']->last_values = isset($res['10c.power.p']) ? $res['10c.power.p'] : array();
        $this->load->view('dashboard_view', $data);
    }

    public function ajaxGetEnergyByHours() {
        if ($this->input->is_ajax_request()) {
            $this->influxdb->setTimePrecision('ms');
            $db = $this->influxdb->getDatabase();

            $start = strtotime('today -31 day');
            $res = $db->query("select * from 1h.energy.w where time > " . $start . "s;");
            if (count($res) > 0) {
                $res = $res['1h.energy.w'];
            } else {
                $res = array();
            }
            echo json_encode($res);
        }
    }

    public function ajaxGetNewValues() {
        if ($this->input->is_ajax_request()) {
            $this->influxdb->setTimePrecision('ms');
            $db = $this->influxdb->getDatabase();
            $row = $db->query('select time, MEAN(value) from 10c.voltage.rms, 10c.current.rms, 10c.power.p, 10c.power.q, 10c.power.pf  limit 1;');
            echo json_encode($row);
        }
    }

    public function ajaxGetHorizonValues() {
        if ($this->input->is_ajax_request()) {
            $this->influxdb->setTimePrecision('ms');
            $db = $this->influxdb->getDatabase();
            //$quer = 'select time, MEAN(value) from ' . $this->input->post('key') . ' where time > ' . $this->input->post('start') . ' AND time < ' . $this->input->post('stop') . ' group by time(' . ($this->input->post('step') / 1000) . 's) limit 500;';
            $row = $db->query('select time, MEAN(value) from ' . $this->input->post('key') . ' where time > ' . $this->input->post('start') . 'ms -3s AND time < ' /* . $this->input->post('stop') . 'ms -3s */ . 'NOW() group by time(' . ($this->input->post('step') / 1000) . 's)  fill(0) order ASC;');
            echo json_encode($row);
        }
    }

    public function ajaxGetMainValues() {
        if ($this->input->is_ajax_request()) {
            $this->load->model('data_model');
            echo json_encode($this->data_model->getMainValues());
        }
    }

}
