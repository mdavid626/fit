<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Description of classroom_model
 * Model implementing xml import/export.
 * @author xmolna02
 */
class Xml_model extends CI_Model {
    
    var $xml_path;
    
    public function __construct()
    {
        parent::__construct();
		
		$this->xml_path = realpath(APPPATH . '../xml');        
    }
    
	public function do_upload() 
    {
        $result = array('data' => '', 'error' => '');
        
		$config = array(
			'allowed_types' => 'xml',
			'upload_path' => $this->xml_path,
			'max_size' => 2000,
            'overwrite' => FALSE
		);
		
		$this->load->library('upload', $config);
        
		if ($this->upload->do_upload())
        {
            // success
            $result['data'] = $this->upload->data();
        }
        else
        {
            // error 
            $result['error'] = $this->upload->display_errors();
        }
 
        return $result;
	}
    
    public function import_xml($full_path)
    {
        $result = array('data' => '', 'status' => 0);
        
        // temporary disable warnings
        error_reporting(E_ALL ^ (E_NOTICE | E_WARNING));
        
        $reader = new XMLReader();
        $stat = $reader->open($full_path);
        
        if (!$stat)
        {
            error_reporting(E_ALL);
            $result['status'] = -1;
            return $result;
        }
        
        $data = array();
        
        while ($reader->read()) 
        {
            if ($reader->nodeType == XMLREADER::ELEMENT) 
            {
                if ($reader->localName == 'Ucebna')
                {
                    $data []= array('id_miest' => $reader->getAttribute('id_miest'), 
                                    'typ_miest' => $reader->getAttribute('typ_miest'), 
                                    'kapacita' => $reader->getAttribute('kapacita'), 
                                    'spec_vyb' => $reader->getAttribute('spec_vyb'));
                }
            } 
        }
        
        $reader->close();

        // delete imported xml file
        unlink($full_path);
        
        error_reporting(E_ALL);
        
        if (count($data) > 0)
        {
            if ($this->input->post('overwrite'))
            {
                $this->db->empty_table('Ucebna'); 
            }
            
            foreach ($data as $item)
            {
                // NOT NULL
                if ($item['id_miest'] == '' || $item['typ_miest'] == '' || $item['kapacita'] == '')
                {
                    $result['data'] = $item;
                    $result['status'] = -3;
                    return $result;
                }
                
                // Primary key                
                $this->db->where('id_miest', $item['id_miest']);
                if ($this->db->get('Ucebna')->result())
                {
                    $result['data'] = $item;
                    $result['status'] = -4;
                    return $result;
                }
            }
            
            // insert data
            $this->db->insert_batch('Ucebna', $data); 
            
            $result['status'] = 1; // 1 means no error
            $result['data'] = count($data);
            return $result;
        }        
        else
        {
            $result['status'] = -2;
            return $result;
        }   
    }
    
    public function export_xml()
    {
        $query = $this->db->get('Ucebna');
        
        $filename = "Ucebna-" . time() . ".xml";
        
        // set headers
        header("Content-type: text/xml");
        header("Content-Description: File Transfer");
        header("Content-Disposition: attachment; filename=\"$filename\"");
        header("Cache-Control: public");
        
        // create xml file
        $writer = new XMLWriter();
        $writer->openURI('php://output');
        $writer->setIndent(true);
        $writer->startDocument('1.0', 'utf-8');
        
        $writer->startElement('Ucebne');
        
            foreach ($query->result() as $row)
            {
                $writer->startElement('Ucebna');

                    $writer->startAttribute('id_miest');
                        $writer->text($row->id_miest);
                    $writer->endAttribute();
                
                    $writer->startAttribute('typ_miest');
                        $writer->text($row->typ_miest);
                    $writer->endAttribute();
                    
                    $writer->startAttribute('kapacita');
                        $writer->text($row->kapacita);
                    $writer->endAttribute();
                    
                    $writer->startAttribute('spec_vyb');
                        $writer->text($row->spec_vyb);
                    $writer->endAttribute();
                    
                $writer->endElement();
            }
            
        $writer->endElement();
        
        $writer->endDocument();
        $writer->flush();
        
        exit();
    }
}

/* End of file xml_model.php */
/* Location: ./application/models/xml_model.php */