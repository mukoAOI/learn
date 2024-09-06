import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Calendar;
import java.util.GregorianCalendar;

public class SwingCalendar extends JFrame {
    private JLabel monthLabel;  
    private JButton prevButton, nextButton;  
    private JPanel calendarPanel; 
    private Calendar calendar;  

    public SwingCalendar() {
        
        setTitle("Swing Calendar");
        
        
        calendar = new GregorianCalendar();

        
        monthLabel = new JLabel("", JLabel.CENTER);
        prevButton = new JButton("<< Previous");
        nextButton = new JButton("Next >>");
        calendarPanel = new JPanel(new GridLayout(0, 7)); 
        JPanel topPanel = new JPanel(new BorderLayout());
        topPanel.add(prevButton, BorderLayout.WEST);
        topPanel.add(monthLabel, BorderLayout.CENTER);
        topPanel.add(nextButton, BorderLayout.EAST);
        prevButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                calendar.add(Calendar.MONTH, -1);  
                updateCalendar();  
            }
        });

        nextButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                calendar.add(Calendar.MONTH, 1); 
                updateCalendar(); 
            }
        });

   
        add(topPanel, BorderLayout.NORTH);
        add(calendarPanel, BorderLayout.CENTER);

       
        setSize(400, 300);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setVisible(true);

   
        updateCalendar();
    }


    private void updateCalendar() {
  
        calendarPanel.removeAll();
        int month = calendar.get(Calendar.MONTH);
        int year = calendar.get(Calendar.YEAR);
        calendar.set(Calendar.DAY_OF_MONTH, 1);  
        int firstDayOfWeek = calendar.get(Calendar.DAY_OF_WEEK);
        int daysInMonth = calendar.getActualMaximum(Calendar.DAY_OF_MONTH);
        monthLabel.setText(calendar.getDisplayName(Calendar.MONTH, Calendar.LONG, getLocale()) + " " + year);
        String[] daysOfWeek = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
        for (String day : daysOfWeek) {
            calendarPanel.add(new JLabel(day, JLabel.CENTER));
        }
        for (int i = 1; i < firstDayOfWeek; i++) {
            calendarPanel.add(new JLabel(""));
        }
        for (int day = 1; day <= daysInMonth; day++) {
            calendarPanel.add(new JLabel(String.valueOf(day), JLabel.CENTER));
        }
        calendarPanel.revalidate();
        calendarPanel.repaint();
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                new SwingCalendar();
            }
        });
    }
}
