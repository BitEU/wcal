# wcal: A Windows Terminal Calendar App

A terminal-based calendar application for Windows, inspired by calcurse. This app provides a text-based interface for managing appointments and todo lists, similar to the Unix/Linux calcurse application but designed specifically for Windows Console.

## Features

- **Three-panel interface**:
  - Left panel: Daily appointments view
  - Center panel: Monthly calendar with navigation
  - Right panel: TODO list with priorities
  
- **Calendar functionality**:
  - Navigate through months and days
  - Visual highlighting of current date
  - Week numbers display
  - Appointments indicator on calendar days

- **Appointment management**:
  - Add appointments with time and duration
  - View daily appointments
  - Edit and delete appointments
  - Automatic sorting by time

- **TODO list**:
  - Add tasks with priority levels (Normal, High, Urgent)
  - Mark tasks as complete/incomplete
  - Sort by priority and completion status
  - Visual indicators for priority and status

- **Data persistence**:
  - Automatic saving of appointments and todos
  - Binary file format for fast loading

## Requirements

- Windows 7 or later
- MSVC compiler (MinGW recommended)
- Windows Console (cmd.exe or Windows Terminal)

## Building

Just run build.bat in a VS Dev CMD windows

## Usage

### Running the application:
```cmd
calcurse.exe
```

### Keyboard shortcuts:

**Navigation:**
- `Arrow Keys`: Navigate calendar/lists
- `Tab`: Switch between panels
- `PgUp/PgDn`: Previous/Next month (calendar) or scroll lists
- `Home`: Jump to today's date

**Actions:**
- `a`: Add appointment (in calendar/appointment view) or todo (in todo view)
- `d`: Delete selected item
- `e`: Edit selected item
- `Space`: Toggle todo completion
- `h`: Show help
- `q`: Quit application

**Calendar Navigation:**
- `Left/Right`: Previous/Next day
- `Up/Down`: Previous/Next week
- `PgUp/PgDn`: Previous/Next month

## File Structure

```
calendar-app/
├── main.c           # Entry point and main loop
├── ui.c/h           # Terminal UI rendering
├── calendar.c/h     # Calendar calculations
├── appointments.c/h # Appointment management
├── todo.c/h         # TODO list management
├── storage.c/h      # File I/O operations
├── input.c/h        # Keyboard input handling
├── build.bat        # Windows build script
├── Makefile         # Make build configuration
└── README.md        # This file
```

## Data Files

The application creates two data files in the current directory:
- `appointments.dat`: Stores all appointments
- `todos.dat`: Stores all TODO items

These files are automatically created on first run and updated when you exit the application.

## Customization

### Colors
Colors can be customized by modifying the color definitions in `ui.h`:
```c
#define BORDER_FG       (COLOR_CYAN | COLOR_BRIGHT)
#define HEADER_FG       (COLOR_YELLOW | COLOR_BRIGHT)
#define TODAY_FG        (COLOR_RED | COLOR_BRIGHT)
```

### Layout
Panel sizes can be adjusted in the `draw_ui()` function in `ui.c`.

## Differences from Unix calcurse

This Windows implementation differs from the original calcurse in several ways:

1. **Console API**: Uses Windows Console API instead of ncurses
2. **File paths**: Uses current directory for data files instead of ~/.calcurse/
3. **Key bindings**: Adapted for Windows console (e.g., function keys work differently)
4. **Visual style**: ASCII box-drawing characters optimized for Windows console

## Troubleshooting

### Build errors:
- Ensure GCC is installed and in your PATH
- Try using MinGW-w64 for better Windows compatibility

### Display issues:
- The app uses code page 437 for box-drawing characters
- Best viewed in Windows Console or Windows Terminal
- Ensure console font supports box-drawing characters (Consolas recommended)

### Data not saving:
- Check write permissions in the current directory
- Ensure you exit with 'q' to save data properly

## Future Enhancements

- [ ] Recurring appointments
- [ ] Import/Export to iCal format
- [ ] Notification system
- [ ] Configuration file support
- [ ] Unicode support for better graphics
- [ ] Mouse support in Windows Terminal

## License

This project is inspired by calcurse (https://github.com/lfos/calcurse) but is a completely new implementation for Windows.

## Contributing

Feel free to submit issues, fork the repository, and create pull requests for any improvements.