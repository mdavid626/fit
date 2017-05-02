<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
require_once 'includes/authtool.php';
require_once 'includes/setuptool.php';

/**
 * Description of Site
 * Controller for controlling search operations
 * @author xmolna02
 */
class Search extends CI_Controller {
    
    
    public function classroom()
    {
        $data = SetupTool::setup_roomsearch();
        $data['submit'] = 'search/classroom';
            //$this->load->model('search_model');
        if (    $data['results'] === 1 && 
                ($query = $this->search_model->search_classroom())  )
        {
            // got some data
                $data['query'] = $query;
        }

        AuthTool::load_template('search_classroom_view', 'Vyhľadať učebne', $data);
    }


    public function reservation()
    {
        $data['results'] = 0;
        $data['query'] = '';

        if ($this->input->post('search_flag'))
        {
            $this->load->library('form_validation');
            
            // because of repopulation... (set_value)
            $this->form_validation->set_rules('room_id', '', '');
            $this->form_validation->set_rules('datum1', '', '');
            $this->form_validation->set_rules('datum2', '', '');
            $this->form_validation->set_rules('by_room_id', '', '');
            $this->form_validation->set_rules('by_datum1', '', '');
            $this->form_validation->set_rules('by_datum2', '', '');
            
            // form validation            
            if ($this->input->post('by_room_id'))
            {
                $this->form_validation->set_rules('room_id', 'identifikačné číslo', 'trim|required|min_length[4]|max_length[4]');
            }
            
            if ($this->input->post('by_datum1'))
            {
                $this->form_validation->set_rules('datum1', 'termín od', 'trim|required|min_length[1]|max_length[20]');
            }
            
            if ($this->input->post('by_datum2'))
            {
                $this->form_validation->set_rules('datum2', 'termín do', 'trim|required|min_length[1]|max_length[20]');
            }
            
            if($this->form_validation->run() == TRUE)
            {
                // validation OK
                $data['results'] = 1;
                
                $this->load->model('search_model');
                
                if ($query = $this->search_model->search_reservation())
                {
                    // got some data
                	$data['query'] = $query;
                }
            }
        }

        AuthTool::load_template('search_reser_view', 'Vyhľadať rezerváciu', $data);
    }
    
    public function schedule()
    {
        $data['results'] = 0;
        $data['query'] = '';
        
        $this->load->model('search_model');
    
        // ucebna dropdown
        $ucebna_dd = array('' => '');
        $query = $this->search_model->get_ucebne();

        foreach ($query->result() as $row)
        {
            $ucebna_dd[$row->id_miest] = $row->id_miest;
        }
        
        $data['ucebna_dd'] = $ucebna_dd;
        
        // obor dropdown
        $obor_dd = array('' => '');
        $query = $this->search_model->get_obor();

        foreach ($query->result() as $row)
        {
            $obor_dd[$row->stud_obor] = $row->stud_obor;
        }
        
        $data['obor_dd'] = $obor_dd;
        
        // rocnik dropdown
        $rocnik_dd = array('' => '');
        $query = $this->search_model->get_rocnik();

        foreach ($query->result() as $row)
        {
            $rocnik_dd[$row->rocnik] = $row->rocnik;
        }
        
        $data['rocnik_dd'] = $rocnik_dd;
        
        if ($this->input->post('post_flag'))
        {
            $this->load->library('form_validation');
            
            // because of repopulation... (set_value)
            $this->form_validation->set_rules('ucebna', '', '');
            $this->form_validation->set_rules('obor', '', '');
            $this->form_validation->set_rules('rocnik', '', '');
            $this->form_validation->set_rules('datum1', '', '');
            $this->form_validation->set_rules('datum2', '', '');
            $this->form_validation->set_rules('by_ucebna', '', '');
            $this->form_validation->set_rules('by_obor', '', '');
            $this->form_validation->set_rules('by_rocnik', '', '');
            $this->form_validation->set_rules('by_datum1', '', '');
            $this->form_validation->set_rules('by_datum2', '', '');
            
            // form validation            
            if ($this->input->post('by_ucebna'))
            {
                $this->form_validation->set_rules('ucebna', 'učebňa', 'trim|required|min_length[4]|max_length[4]');
            }
            
            if ($this->input->post('by_obor'))
            {
                $this->form_validation->set_rules('obor', 'obor', 'trim|required|min_length[1]|max_length[100]');
            }
            
            if ($this->input->post('by_rocnik'))
            {
                $this->form_validation->set_rules('rocnik', 'rocnik', 'trim|required|min_length[1]|max_length[10]');
            }

            if ($this->input->post('by_datum1'))
            {
                $this->form_validation->set_rules('datum1', 'termín od', 'trim|required|min_length[1]|max_length[20]');
            }
            
            if ($this->input->post('by_datum2'))
            {
                $this->form_validation->set_rules('datum2', 'termín do', 'trim|required|min_length[1]|max_length[20]');
            }
            
            if($this->form_validation->run() == TRUE)
            {
                // validation OK
                $data['results'] = 1;
                $data['obor_h'] = !$this->input->post('by_obor');
                $data['rocnik_h'] = !$this->input->post('by_rocnik');
                $data['ucebna_h'] = !$this->input->post('by_ucebna');
                
                $this->load->model('search_model');
                
                if ($query = $this->search_model->search_schedule())
                {
                    // got some data
                	$data['query'] = $query;
                }
            }
        }

        AuthTool::load_template('schedule_view', 'Rozvrh', $data);
    }
   
}

/* End of file search.php */
/* Location: ./application/controllers/search.php */