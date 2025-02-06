/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *\
 * Part of BlueTit Solver, licensed under Apache 2.0 with Commons Clause.
 * Commercial use, including SaaS, requires a separate license, see /LICENSE.md
\* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

import { FC } from "react";
import { AiOutlinePython as PythonIcon } from "react-icons/ai";
import {
  Activity as ActivityIcon,
  Database as DatabaseIcon,
  Settings as SettingsIcon,
  Sliders as SlidersIcon,
  Terminal as TerminalIcon,
} from "react-feather";

import { BottomMenu, LeftMenu, MenuItem } from "./MainMenu";
import { PythonShell } from "./PythonShell";
import { TreeView, MockData } from "./TreeView";
import { Viewer } from "./Viewer";

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

export const App: FC = () => {
  const leftIconSize = 24;
  const bottomIconSize = 16;
  return (
    <div className="h-screen w-screen flex flex-row select-none text-sm">
      <LeftMenu>
        <MenuItem
          name="Configuration"
          icon={<SlidersIcon size={leftIconSize} />}
          group={0}
        >
          <TreeView nodes={MockData} />
        </MenuItem>
        <MenuItem
          name="Storage"
          icon={<DatabaseIcon size={leftIconSize} />}
          group={0}
        />
        <MenuItem
          name="Activity"
          icon={<ActivityIcon size={leftIconSize} />}
          group={1}
        />
        <MenuItem
          name="Settings"
          icon={<SettingsIcon size={leftIconSize} />}
          group={1}
        />
      </LeftMenu>
      <div className="flex-1 flex flex-col">
        <Viewer />
        <BottomMenu>
          <MenuItem
            name="Python shell"
            icon={<PythonIcon size={bottomIconSize} />}
            group={0}
          >
            <PythonShell />
          </MenuItem>
          <MenuItem
            name="Console"
            icon={<TerminalIcon size={bottomIconSize} />}
            group={0}
          />
        </BottomMenu>
      </div>
    </div>
  );
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
