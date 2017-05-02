<?php

/**
 * Description of SetupTool
 * Functions for setting up data for pages and other reusable methods
 * This class is not intended for instantiation
 * @author xmesza03
 */
class SetupTool {
    
    // no instantiation is permitted
    private function __construct() {
        ;
    }
    
    /**
     * Sets up a data structure to be passed to the search classroom view
     * @return array Returns an array with the constructed data structure 
     */
    public function setup_roomsearch() 
    {
        $data['results'] = 0;
        $data['query'] = '';
        
        // typ miesta dropdown
        $this->load->model('search_model');
        
        $typ_m_dd = array('' => '');
        $q = $this->search_model->get_typ_miesta();

        foreach ($q->result() as $row)
        {
            $typ_m_dd[$row->typ_miest] = $row->typ_miest;
        }
        
        $data['typ_miesta_dd'] = $typ_m_dd;
            
        
        if ($this->input->post('search_flag'))
        {
            $this->load->library('form_validation');
            
            // because of repopulation... (set_value)
            $this->form_validation->set_rules('room_id', '', '');
            $this->form_validation->set_rules('miesto', '', '');
            $this->form_validation->set_rules('kapacita', '', '');
            $this->form_validation->set_rules('vybavenie', '', '');
            $this->form_validation->set_rules('datum1', '', '');
            $this->form_validation->set_rules('datum2', '', '');
            $this->form_validation->set_rules('by_room_id', '', '');
            $this->form_validation->set_rules('by_miesto', '', '');
            $this->form_validation->set_rules('by_kapacita', '', '');
            $this->form_validation->set_rules('by_vybavenie', '', '');
            $this->form_validation->set_rules('by_datum1', '', '');
            $this->form_validation->set_rules('by_datum2', '', '');
            
            // form validation            
            if ($this->input->post('by_room_id'))
            {
                $this->form_validation->set_rules('room_id', 'identifikačné číslo', 'trim|required|min_length[4]|max_length[4]');
            }
            
            if ($this->input->post('by_miesto'))
            {
                $this->form_validation->set_rules('miesto', 'miesto', 'trim|required|min_length[1]|max_length[100]');
            }
            
            if ($this->input->post('by_kapacita'))
            {
                $this->form_validation->set_rules('kapacita', 'min. kapacita', 'trim|required|min_length[1]|max_length[10]');
            }
            
            if ($this->input->post('by_vybavenie'))
            {
                $this->form_validation->set_rules('vybavenie', 'vybavenie', 'trim|required|min_length[1]|max_length[100]');
            }
            
            if ($this->input->post('by_datum1'))
            {
                $this->form_validation->set_rules('datum1', 'volné od', 'trim|required|min_length[1]|max_length[20]');
            }
            
            if ($this->input->post('by_datum2'))
            {
                $this->form_validation->set_rules('datum2', 'volné od', 'trim|required|min_length[1]|max_length[20]');
            }
            
            if($this->form_validation->run() == TRUE)
            {
                // validation OK

                $data['results'] = 1;
                
            }
        }

        return $data;
    }
    
    /**
     * Sets up a data structure to be passed to the edit classroom view
     * @param bool $is_addition
     * @return array Returns an array with the constructed data structure
     */
    public function setup_roomedit($is_addition)
    {
        $data['activate'] = FALSE;
        if ($this->input->post('add_flag'))
        {
            $this->load->library('form_validation');
            if($is_addition)
                $this->form_validation->set_rules('room_id', 'identifikačné číslo', 'trim|required|alpha_numeric|min_length[4]|max_length[4]');
            else $this->form_validation->set_rules('room_id', 'identifikačné číslo', 'trim|min_length[4]|max_length[4]');
            $this->form_validation->set_rules('room_capacity', 'kapacita', 'trim|required|numeric|min_length[1]|max_length[10]');
            $this->form_validation->set_rules('room_equip', 'vybavenie', 'trim|min_length[1]|max_length[100]');
            $this->form_validation->set_rules('room_type', 'typ miestnosti', 'trim|required|min_length[1]|max_length[100]');
            if($this->form_validation->run() == TRUE)
            {
               $data['activate'] = TRUE;
               $data['values'] = array(
                              'id_miest' => $this->input->post('room_id'),
                              'typ_miest' => $this->input->post('room_type'),
                              'kapacita' => $this->input->post('room_capacity'),
                              'spec_vyb' => $this->input->post('room_equip')
                            );

            }
            return $data;
        }
    }
}

?>
